/*******************************************************************************
* File Name:   oled.h (WMS v4.1)
* Description: implementasi task oled display (ssd1306)
* Programmer: Bostang
* Tanggal : 07 April 2024, 29 Mei 2024
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
#define OLED_VER				(64)

#define OLED_DELAY_MS		(1000)
#define OLED_DELAY_STATE_MS		(500)  // durasi menampilkan tulisan suatu STATE

const GUI_FONT * OldFont;

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

    // variabel lokal (tidak dibagikan dengan task lain)
    uint8_t epc[len_epc];

    tags tagOLED;
	tagOLED.tag_count = 0;

	// inisiasi flag untuk multiple-cycle scanning
	bool flag_oled_display = false; // agar state hanya tampil sekali -> di setiap state, kondisi pemeriksaan bergantian (if !flag.., if flag..)
	bool flag_epc_scanned = false; //menyatakan apakah sudah ada epc yang terbaca atau belum

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

	// menampilkan logo inventrix di awal selama 1 detik
	GUI_DrawBitmap(&bmBMP_inventrix_logo,0,0);

	// delay 1 detik
	vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;

	// menampilkan tulisan state IDLE selama 1 detik
    GUI_Clear();
	GUI_DrawBitmap(&bmstate_idle,0,0);

	// delay 1 detik
	vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;
	for(;;)
	{
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

		            printf("State SCAN\r\n");

		            // beri notifikasi ke data_acquisition_task untuk mulai scanning
		            	// hal ini krusial untuk sinkronisasi task ketika menggunakan RFID scanner
		            // TO-DO
		            xTaskNotifyGive(data_acq_taskHandle);

		            flag_oled_display = false;
		        }

		        if (xQueueReceive(dataQueue, &epc, portMAX_DELAY) == pdPASS)
		        {
		            printf("menerima queue dari task data acquisition\r\n");

		            cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)epc, &epc_length);

		            memcpy(tagOLED.epc_array[tagOLED.tag_count], epc, len_epc * sizeof(uint8_t));

					printf("\r\nEPC terdeteksi!\r\n");
					tagOLED.tag_count++;
					printf("tag count : %d\r\n", tagOLED.tag_count);

		            // Lock the mutex before accessing the shared state
					if (xSemaphoreTake(xStateMutex, portMAX_DELAY) == pdTRUE)
					{
						// AKSI di dalam mutex
						*state = STATE_DISPLAY; // langsung pindah ke state display

		                // Release the mutex after accessing the shared state
		                xSemaphoreGive(xStateMutex);
					}
		        }
		        break;

		    case STATE_DISPLAY:
	            printf("flag_oled_display: %d\r\n",flag_oled_display);
		        if (!flag_oled_display)
		        {
		            printf("STATE DISPLAY\r\n");

		            GUI_GotoX(4);

		            printf("menampilkan pada OLED\r\n");
//		            GUI_GotoX(4);
//		            for (int k = 0; k < TX_BUF_SIZE; k++)
//		            {
//		                GUI_DispChar(epc[k]);
//		            }
//		            GUI_DispString("\n");

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

//		            flag_epc_displayed = true;
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
		            GUI_GotoX(4);
		            GUI_DispString("SENDING DATA...\n");

		//            for (int k = 0; k < tagOLED.tag_count; k++) {
		//                cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)tagOLED.epc_array[k], &epc_length);
		//                printf("\r\n");
		//            }

		            // menampilkan tulisan state SEND selama 1/2 detik
					GUI_Clear();
					GUI_DrawBitmap(&bmstate_send,0,0);

					// delay 1 detik
					vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_STATE_MS)) ;
					GUI_Clear();


		            // mengirimkan queue ke http task
		            if (xQueueSend(dataQueue_array, &tagOLED, portMAX_DELAY) != pdPASS) {
		                printf("Failed to send data to queue.\r\n");
		            }

		            // Memberi notifikasi pada task HTTP untuk mulai kirim data
		//            xTaskNotifyGive(client_task_handle);
		            vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_MS)); // blok task OLED supaya dilanjut task http

		            flag_oled_display = false;
		        }
		        break;

		    default:
		        GUI_DispString("State ERROR");
		        break;
		}

		if (true == gpio_intr_flag_mode) {
		    // menampilkan logo tengkorak awal
		    GUI_DrawBitmap(&bmBMP_skull, 0, 0);

		    // delay 1 detik
		    vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_MS));

		    // membersihkan layar
		    GUI_Clear();

		    // menampilkan countdown
		    OldFont = GUI_SetFont(&GUI_FontD64); // Buffer old font; mengubah font

		    for (int k = 10; k > 0; k--) {
		        GUI_DispDecAt(k, 16, 0, 2); // Disp value
		        vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_MS));
		    }
		}

		taskYIELD(); // Yield to other tasks


//		// Mendeskripsikan perilaku OLED pada masing-masing state (IDLE, SCAN, DISPLAY, SEND)
//		if (*state == STATE_IDLE)
//		{
//			if (!flag_oled_display)
//			{
//				// inisiasi ulang flag untuk multiple-cycle scanning
//				flag_epc_displayed = false;// menyatakan apakah tag sudah ditampilkan pada OLED/belum
//				// membersihkan layar
//				GUI_Clear();
//				GUI_GotoX(4);
////				GUI_DispString("State IDLE\n");
//				flag_oled_display = true;
//			}
//		}
//
//		else if (*state == STATE_SCAN)
//		{
//			// menampilkan pada OLED
//			if (flag_oled_display) // selang-seling
//			{
//				GUI_GotoX(4);
////				GUI_DispString("State SCAN\n");
//				flag_oled_display = false;
////				flag_epc_scanned = false; //menyatakan apakah sudah ada epc yang terbaca atau belum
//
//			}
//
//            if (xQueueReceive(dataQueue, &epc, portMAX_DELAY) == pdPASS)
//            {
//				printf("menerima queue dari task data acquisition\r\n");
//
//				cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)epc, &epc_length);
//
//            	memcpy(tagOLED.epc_array[tagOLED.tag_count], epc, len_epc * sizeof(uint8_t));
//
////            	printf("menerima queue dari task data acquisition\r\n");
//            	// melakukan scanning dengan RFID
////				if (!flag_epc_scanned)
////				{
//
////                uint32_t state_crit = cyhal_system_critical_section_enter();
//					// menampilkan epc ke terminal
////					cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)epc, &epc_length);
//
////            	// menyimpan epc pada variabel lokal (khawatir dimodifiksai task lain)
////            	uint8_t epc_local;
////            	// Copy elements one by one from epc to epc_array[tag_count]
////				for (int i = 0; i < len_epc; i++)
////				{
////					epc_array[tag_count][i] = epc[i];
////				}
////				vTaskDelay(pdMS_TO_TICKS(100)); // Debouncing delay
//
//
//					// Copy elements one by one from epc to epc_array[tag_count]
////					for (int i = 0; i < len_epc; i++)
////					{
////						epc_array[tag_count][i] = epc[i];
////					}
//
//					printf("\r\nEPC terdeteksi!\r\n");
////					flag_epc_scanned = true;
//					tagOLED.tag_count++;
//					printf("tag count : %d\r\n",tagOLED.tag_count);
////				cyhal_system_critical_section_exit(state_crit);
////					cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)&epc_array[0], &epc_length);
//
//					*state = STATE_DISPLAY; // langsung pindah ke state display
////				}
//            }
//
//		}
//
//		else if (*state == STATE_DISPLAY)
//		{
//			if (!flag_oled_display)
//			{
//				printf("STATE DISPLAY\r\n");
//
//				GUI_GotoX(4);
////				GUI_DispString("State DISPLAY\n");
//
//
//				printf("menampilkan pada OLED\r\n");
//				GUI_GotoX(4);
//				for (int k = 0;k<TX_BUF_SIZE;k++)
//				 {
//					GUI_DispChar(epc[k]);
//				 }
//				GUI_DispString("\n");
//
//				flag_epc_displayed = true;
////				*state = STATE_SEND; // langsung pindah ke state display
//				*state = STATE_IDLE; // langsung pindah ke state display
//
//			}
//			flag_oled_display = true;
//		}
//
//		else if (*state == STATE_SEND)
//		{
//			if (flag_oled_display)
//			{
//				GUI_GotoX(4);
////				GUI_DispString("State SEND\n");
//				GUI_DispString("SENDING DATA...\n");
//
//
////				for (int k = 0;k<tagOLED.tag_count;k++)
////				{
////					cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)tagOLED.epc_array[k], &epc_length);
////					printf("\r\n");
////				}
//
//				// mengirimkan queue ke http task
//				if (xQueueSend(dataQueue_array, &tagOLED, portMAX_DELAY) != pdPASS)
//				{
//					printf("Failed to send data to queue.\r\n");
//				}
//
//				// Memberi notifikasi pada task HTTP untuk mulai kirim data
////			    xTaskNotifyGive(client_task_handle);
//				vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_MS)) ; // blok task OLED supaya dilanjut task http
//
//
//				flag_oled_display = false;
//			}
//		}
//		else
//		{
//			GUI_DispString("State ERROR");
//		}
//
//		if (true == gpio_intr_flag_mode)
//		{
//			// menampilkan logo tengkorak awal
//			GUI_DrawBitmap(&bmBMP_skull,0,0);
//
//			// delay 1 detik
//			vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_MS)) ;
//
//			// membersihkan layar
//			GUI_Clear();
//
//			// menampilkan countdown
//			OldFont = GUI_SetFont(&GUI_FontD64); // Buffer old font; mengubah font
//
//			for (int k = 10;k>0;k--)
//			{
//				GUI_DispDecAt(k,16,0,2); // Disp value
//				vTaskDelay(pdMS_TO_TICKS(OLED_DELAY_MS)) ;
//			}
//		}
//	    taskYIELD(); // Yield to other tasks
	}
}

#endif // FSM_H

/* [] END OF FILE */
