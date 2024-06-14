/*******************************************************************************
* File Name:   oled.h (WMS v4.1)
* Description: implementasi task oled display (ssd1306)
* Programmer: Bostang
* Tanggal : 07 April 2024, 29 Mei 2024, 11 Juni 2024
*******************************************************************************/

// HEADER GUARD
#ifndef OLED_H
#define OLED_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cybsp.h"
#include "mtb_ssd1306.h"
#include "GUI.h"
#include "cyhal.h"
#include "cyabs_rtos.h"

#include "utils.h"
#include "oled_img.h"
#include "fsm.h"
#include "data_acquisition.h"
#include "utils.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define OLED_TASK_NAME         	("OLED Task")
#define OLED_TASK_NAME         	("OLED Task")
#define OLED_TASK_STACK_SIZE   	(1024)
#define OLED_TASK_PRIORITY     	(1)

#define OLED_HOR				(128)
#define OLED_WIDTH				(OLED_HOR)
#define OLED_HEIGHT				(OLED_VER)
#define OLED_VER				(64)

#define OLED_DELAY_MS		(1000)
#define OLED_DELAY_STATE_MS		(500)  // durasi menampilkan tulisan suatu STATE

const GUI_FONT * OldFont;

// Define a buffer large enough to hold the bitmap data
#define BYTES_PER_PIXEL				(1)
#define BUFFER_SIZE (OLED_HOR * OLED_VER * BYTES_PER_PIXEL)
uint8_t displayBuffer[BUFFER_SIZE];

/*******************************************************************************
* Global Variables
*******************************************************************************/

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void oled_task(void *pvParameters);

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: oled_task
********************************************************************************
* Summary:
* Mendeskripsikan perilaku OLED berdasarkan FSM
*
* Parameters:
*  void* context passed from main function
*
* Return:
*  void
*
*******************************************************************************/
void oled_task(void *pvParameters)
{
    TaskParameters *params = (TaskParameters *)pvParameters;
    int* state = &(params->state);
    bool* scanner_selector = &(params->scanner_mode);

    // variabel lokal (tidak dibagikan dengan task lain)
    uint8_t epc[len_epc];

    tags tagOLED;
	tagOLED.tag_count = 0;
    allocate_epc_memory(&tagOLED);


	// inisiasi flag untuk multiple-cycle scanning
	bool flag_oled_display = false; // agar state hanya tampil sekali -> di setiap state, kondisi pemeriksaan bergantian (if !flag.., if flag..)
	bool flag_epc_scanned = false; //menyatakan apakah sudah ada epc yang terbaca atau belum
	bool flag_sukses_kirim;	// menyatakan sukses kirim (diterima dari http_client_task)
	bool flag_sukses_pindai; // menyatakan sukses pindai (berdasarkan hasil dari data_acquisition_task)
	bool flag_pindai_unik; // menyatakan hasil pindaian unik (dibandingkan terhadap semua tag di tagOLED.epc_array)
	bool flag_mode_display; // agar perubahan mode (inbound/outbound) tampil pada OLED

	/* Inisiasi block I2C */
	cy_rslt_t result;
	cyhal_i2c_t i2c_obj;

	result = cyhal_i2c_init(&i2c_obj, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
	handle_error(result);

	/* Inisiasi library OLED display */
	result = mtb_ssd1306_init_i2c(&i2c_obj);
	handle_error(result);

	/* Inisiasi Library emWin */
	GUI_Init();

	// menampilkan logo inventrix di awal selama 1/2 detik
	GUI_DrawBitmap(&bmBMP_inventrix_logo,0,0);

	// delay 1 detik
	vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;

	// menampilkan tulisan state IDLE selama 1/2 detik
    GUI_Clear();
	GUI_DrawBitmap(&bmstate_idle,0,0);

	// delay 1 detik
	vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;
	for(;;)
	{
		// pindah mode inbound-outbound
        if (gpio_intr_flag_mode) {flag_mode_display = true;}
        while (flag_mode_display)
		  {
			  if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
			  {
				  // Mengambil mutex sebelum mengakses resource bersama
				  if (xSemaphoreTake(xModeMutex, portMAX_DELAY) == pdTRUE)
				  {
					  // mengirim notifikasi ke Buzzer (beep 2 kali tanda mode berubah (inbound <-> outbound))
					  xTaskNotifyGive(buzzer_taskHandle);

					  GUI_Clear();
					  if (strcmp(ANYTHINGRESOURCE, "/hw-crud/inbound") == 0)
					  {
						  // mencetak mode sekarang ini
						  GUI_DrawBitmap(&bminbound_mode,0,0);
					  }
					  else if (strcmp(ANYTHINGRESOURCE, "/hw-crud/outbound") == 0)
					  {
						  GUI_DrawBitmap(&bmoutbound_mode,0,0);
					  }

					  // Lepaskan mutex setelah selesai
					  xSemaphoreGive(xModeMutex);

					  // delay 1/2 detik
					  vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS));
					  GUI_Clear();

					  // melakukan restore display
					  if (tagOLED.tag_count == 0)
					  {
						  GUI_DrawBitmap(&bmstate_idle,0,0);
						  vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS));
					  }
					  else
					  {
						  for (int j = 0; j < tagOLED.tag_count; j++)
						  {
							  GUI_GotoX(4);
							  for (int k = 0; k < TX_BUF_SIZE; k++)
							  {
								  GUI_DispChar(tagOLED.epc_array[j][k]);
							  }
							  GUI_DispString("\n");
						  }
					  }

					  flag_mode_display = false;
				  }
				  else
				  {
					  // Handle error jika tidak dapat mengambil mutex
					  printf("Failed to take mutex.\r\n");
				  }
			  }
		  }

		// Mendeskripsikan perilaku OLED pada masing-masing state (IDLE, SCAN, DISPLAY, SEND)
		switch (*state)
		{
		    case STATE_IDLE:

		        if (!flag_oled_display)
		        {
		            // inisiasi ulang flag untuk multiple-cycle scanning
		            // membersihkan layar
//		            GUI_Clear();
//		            GUI_GotoX(4);
		//            GUI_DispString("State IDLE\n");
		            flag_oled_display = true;
		        }
		        break;

		    case STATE_SCAN:
		        // menampilkan pada OLED
		        if (flag_oled_display)
		        { // selang-seling
//		            GUI_GotoX(4);
//		            GUI_DispString("State SCAN\n");

		        	// menampilkan tulisan state SCAN selama 1/2 detik
		            GUI_Clear();
		        	GUI_DrawBitmap(&bmstate_scan,0,0);

		        	// delay 1/2 detik
		        	vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;
		            GUI_Clear();
		            // menampilkan tulisan "scanning..."
		        	GUI_DrawBitmap(&bmscanning,0,0);

		            // beri notifikasi ke data_acquisition_task untuk mulai scanning
		            	// hal ini krusial untuk sinkronisasi task ketika menggunakan RFID scanner
		            xTaskNotifyGive(data_acq_taskHandle);

		            flag_oled_display = false;
		        }

		        if (xQueueReceive(dataQueue, &epc, portMAX_DELAY) == pdPASS)
		        {
		        	// kalau epc yang diterima nol semua ,maka kembali lagi ke state IDLE
		        		// cek nilai pertama saja
		        	if (epc[0] == '0')
		        	{
		        		// kembali ke state IDLE
						// menampilkan tulisan lakukan scan lagi selama 1/2 detik
		        		GUI_Clear();
						GUI_DrawBitmap(&bmtry_scan_again,0,0);
						vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;

						flag_sukses_pindai = false;

						if (xSemaphoreTake(xStateMutex, portMAX_DELAY) == pdTRUE)
						{
							// AKSI di dalam mutex
							*state = STATE_IDLE; // langsung pindah ke state display

			                // Release the mutex after accessing the shared state
			                xSemaphoreGive(xStateMutex);
						}
		        	}
		        	else
					{
						flag_sukses_pindai = true;
					}

		        	// memeriksa apakah tag yang baru sampai sudah ada di array_epc atau belum
		        	if (is_tag_registered(tagOLED.epc_array,tagOLED.tag_count,epc))
					{
		        		// menampilkan bahwa tag sudah ada di array (tidak unik)
						GUI_Clear();
						GUI_DrawBitmap(&bmtag_existed,0,0);
						vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;
		        		flag_pindai_unik = false;
					}
		        	else
		        	{
		        		flag_pindai_unik = true;
		        	}

		            GUI_Clear(); // bersihkan layar saat terima data


		            cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)epc, &epc_length);

		            if (flag_sukses_pindai && flag_pindai_unik) // apabila tag valid dan unik
		            {
		            	// simpan ke array epc dan inkremen jumlah tag
		            	memcpy(tagOLED.epc_array[tagOLED.tag_count], epc, len_epc * sizeof(uint8_t));
						tagOLED.tag_count++;
		            }
		            printf("\r\ntag count : %d\r\n", tagOLED.tag_count);

		            // menerima notifikasi dari task buzzer
					if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
					{
						printf("berhasil terima notifikasi dari task buzzer\r\n");	// DEBUG
						// Lock the mutex before accessing the shared state
						if (xSemaphoreTake(xStateMutex, portMAX_DELAY) == pdTRUE)
						{
							// AKSI di dalam mutex
							*state = STATE_DISPLAY; // langsung pindah ke state display

							// Release the mutex after accessing the shared state
							xSemaphoreGive(xStateMutex);
						}
					}
		        }
		        break;

		    case STATE_DISPLAY:
		        if (!flag_oled_display)
		        {
		            printf("STATE DISPLAY\r\n");

		            GUI_GotoX(4);

		            printf("menampilkan pada OLED\r\n");

		            // menampilkan keseluruhan tag yang tersimpan di tagOLED.tag_count
					for (int j = 0;j<tagOLED.tag_count;j++)
		            {
					  GUI_GotoX(4);
						for (int k = 0; k < TX_BUF_SIZE; k++)
						{
							GUI_DispChar(tagOLED.epc_array[j][k]);
						}
						GUI_DispString("\n");
		            }

		            // Lock the mutex before accessing the shared state
					if (xSemaphoreTake(xStateMutex, portMAX_DELAY) == pdTRUE)
					{
						// AKSI di dalam mutex
			            *state = STATE_IDLE; // kembali ke state IDLE

		                // Release the mutex after accessing the shared state
		                xSemaphoreGive(xStateMutex);
					}
		        }
		        flag_oled_display = true;
		        break;

		    case STATE_SEND:
		        if (flag_oled_display)
		        {
		            // menampilkan tulisan state SEND selama 1/2 detik
					GUI_Clear();
					GUI_DrawBitmap(&bmstate_send,0,0);

					// delay 1 detik
					vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;
					GUI_Clear();
					// menampilkan tulisan "scanning..."
					GUI_DrawBitmap(&bmsending,0,0);


		            // mengirimkan queue ke http task
		            if (xQueueSend(dataQueue_array, &tagOLED, portMAX_DELAY) != pdPASS)
		            {
		                printf("Failed to send data to queue.\r\n");
		            }

		            vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_MS)); // blok task OLED supaya dilanjut task http

		            // menerima queue dari task HTTP yang menyatakan apakah data sukses dikirim atau tidak
		            	// untuk kemudian ditampilkan pada OLED
		            	// TO-DO
		            if (xQueueReceive(ackQueue, &flag_sukses_kirim, portMAX_DELAY) == pdPASS)
		            {
		            	// mengubah tampilan OLED berdasarkan apakah sukses kirim/tidak
		            	if (flag_sukses_kirim)
		            	{
							GUI_Clear();
							// menampilkan tulisan "data sent complete"
							GUI_DrawBitmap(&bmsent_success,0,0);
		            	}
		            	else
		            	{
							GUI_Clear();
							// menampilkan tulisan "data sent fail"
							GUI_DrawBitmap(&bmsent_fail,0,0);
		            	}

		            	// mengosongkan kembali array epc
		            	tagOLED.tag_count = 0;
		            	free_epc_memory(&tagOLED);
		                allocate_epc_memory(&tagOLED); // mengalokasikan ulang untuk iterasi berikutnya //TO-DO

		                // mengirim notifikasi ke Buzzer (beep 3 kali tanda sudah selesai kirim)
						xTaskNotifyGive(buzzer_taskHandle);

						// setelah menerima notifikasi balik dari buzzer:
						if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
						{
							// kembali ke state IDLE
							vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;
							GUI_Clear();
							// menampilkan tulisan state IDLE selama 1/2 detik
							GUI_Clear();
							GUI_DrawBitmap(&bmstate_idle,0,0);
							vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;
							GUI_Clear();
							if (xSemaphoreTake(xStateMutex, portMAX_DELAY) == pdTRUE)
							{
								// AKSI di dalam mutex
								*state = STATE_IDLE; // langsung pindah ke state display

								// Release the mutex after accessing the shared state
								xSemaphoreGive(xStateMutex);
							}
						}
		            }

		            flag_oled_display = false;
		        }
		        break;

		    default:
		        GUI_DispString("State ERROR");
		        break;
		}

		taskYIELD(); // Yield to other tasks
	}
}

#endif // OLED_H

/* [] END OF FILE */
