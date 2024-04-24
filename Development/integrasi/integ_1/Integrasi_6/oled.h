/*******************************************************************************
* File Name:   oled.h (Integrasi 1)
* Description: implementasi task oled display (ssd1306)
* Programmer: Bostang
* Tanggal : 2024/04/07
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
//#include "fsm.h"
#include "cybsp.h"
#include "mtb_ssd1306.h"
#include "GUI.h"
#include "cyhal.h"

#include "utils.h"
#include "oled_img.h"

#include "fsm.h"

#include "cyabs_rtos.h"
#include "data_acquisition.h"


/*******************************************************************************
* Macros
********************************************************************************/
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_STACK_SIZE   (1024)
//#define OLED_TASK_PRIORITY     (5)
#define OLED_TASK_PRIORITY     (4)		// diturunkan prioritasnya untuk dilihat apakah akan mengalah masalah mutex dengan rfid_task

#define OLED_HOR		(128)
#define OLED_VER		(64)

#define BLINKING_DELAY_MS		(1000)

const GUI_FONT * OldFont;

#define len_epc 12
//const char* epc_test[len_epc]= {"A0", "04", "01", "89", "01", "D1", "A0", "04", "08", "89", "31", "D1"};

/*******************************************************************************
* Global Variables
*******************************************************************************/
bool flag_epc_displayed = 0;// menyatakan apakah tag sudah ditampilkan pada OLED/belum

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
//void oled_task(void* arg);
//void oled_task(void* state);
//void oled_task(int* state);
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
//void oled_task(void* state)
//void oled_task(int* state)
void oled_task(void *pvParameters)
{
    TaskParameters *params = (TaskParameters *)pvParameters;
    int* state = &(params->state);

	uint8_t tx_buf[TX_BUF_SIZE];
	size_t tx_length = TX_BUF_SIZE;
	uint8_t variabel_kirim[TX_BUF_SIZE];

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

		// membersihkan layar
		GUI_Clear();

		// menampilkan logo inventrix di awal selama 2 detik
		GUI_DrawBitmap(&bmBMP_inventrix_logo,0,0);

		// delay 2 detik
		vTaskDelay(pdMS_TO_TICKS(2*BLINKING_DELAY_MS)) ;



		for(;;)
		{
//			if (true == gpio_intr_flag_acq)
//			{
				// atur font
//				OldFont = GUI_SetFont(&GUI_Font8x8_1); // Buffer old font; mengubah font

				if (*state == STATE_IDLE)
				{
//					GUI_DispString("State IDLE");
					printf("State IDLE\r\n");

				}

				else if (*state == STATE_SCAN)
				{
//					GUI_DispStringInRect("State SCAN", &rClient, GUI_TA_HCENTER | GUI_TA_VCENTER);
//					GUI_DispString("State SCAN");
					printf("State scan\r\n");
					flag_epc_displayed = false;

					// melakukan scanning
					uint8_t* resultArray = malloc(TX_BUF_SIZE * sizeof(uint8_t));

					resultArray = data_acquisition_task();
					if (resultArray != NULL)
						{
						   // Copy the contents of the array to variabel_kirim
						   memcpy(variabel_kirim, resultArray, TX_BUF_SIZE * sizeof(uint8_t));

						   // Don't forget to free the memory allocated by the function
						   free(resultArray);
						}
					// menyalin dari variabel kirim ke tx_buf
						for (size_t k = 0;k<TX_BUF_SIZE;k++)
						{
							tx_buf[k] = variabel_kirim[k];
						}
					// menuliskan tx_buf ke terminal
					   cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)tx_buf, &tx_length);
				   //    printf("%s\r\n",tx_buf);


				}
				else if (*state == STATE_DISPLAY)
				{
					printf("state display\r\n");

					// konversi epc ke string lalu tampilkan ke oled
					char char_buf[tx_length + 1];  // Extra space for null terminator

					// Convert each element of tx_buf to char
					for (size_t i = 0; i < tx_length; i++) {
					    char_buf[i] = (char)tx_buf[i];
					}

					// Add null terminator
					char_buf[tx_length] = '\0';

//					// Determine the length of the array
//					size_t tx_buf_len = sizeof(tx_buf) / sizeof(tx_buf[0]);


					//					GUI_DispStringInRect("State IDLE", &rClient, GUI_TA_HCENTER | GUI_TA_VCENTER);
					//					GUI_DispString("State IDLE\n");
					//					GUI_DispHex(epc_test,len_epc);
									    // Lock the mutex before accessing/modifying epc
									    cy_rtos_mutex_get(&epc_mutex, CY_RTOS_NEVER_TIMEOUT);
										if (!flag_epc_displayed) // menampilkan epc sekali
										{
					//						GUI_SetTextAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);
					//						GUI_SetTextAlign(GUI_TA_HCENTER);
											GUI_GotoX(4);
											// Display the char array on OLED
												GUI_DispString(char_buf);


//											for (int k = 0;k<len_epc;k++)
//											{
//					//							GUI_DispString(epc_test[k]);
//												GUI_DispString(params->epc[k]);
//												if (k==5)
//												{
//													GUI_DispString("\n");
//													GUI_GotoX(4);
//												}
//												else
//												{
//													GUI_DispString(" ");
//												}
//											}
											flag_epc_displayed = true;
										}
										 // Unlock the mutex after done with epc
										cy_rtos_mutex_set(&epc_mutex);



//					GUI_DispStringInRect("State DISPLAY", &rClient, GUI_TA_HCENTER | GUI_TA_VCENTER);
//					GUI_DispString("State DISPLAY");

//					// menampilkan epc hasil scan rfid
//					if (!flag_epc_displayed) // menampilkan epc sekali
//					{
////						GUI_SetTextAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);
////						GUI_SetTextAlign(GUI_TA_HCENTER);
//						GUI_GotoX(4);
//						for (int k = 0;k<len_epc;k++)
//						{
////							GUI_DispString(epc_test[k]);
//							GUI_DispString(params->epc[k]);
//							if (k==5)
//							{
//								GUI_DispString("\n");
//								GUI_GotoX(4);
//							}
//							else
//							{
//								GUI_DispString(" ");
//							}
//						}
//						flag_epc_displayed = true;
//					}
				}
				else if (*state == STATE_SEND)
				{
//					GUI_DispStringInRect("State SEND", &rClient, GUI_TA_HCENTER | GUI_TA_VCENTER);
					GUI_DispString("State SEND");
				}
				else
				{
//					GUI_DispStringInRect("State ERROR!", &rClient, GUI_TA_HCENTER | GUI_TA_VCENTER);
					GUI_DispString("State ERROR");
				}
//			}
			if (true == gpio_intr_flag_mode)
			{
				// menampilkan logo tengkorak awal
				GUI_DrawBitmap(&bmBMP_skull,0,0);

				// delay 1 detik
				vTaskDelay(pdMS_TO_TICKS(BLINKING_DELAY_MS)) ;

				// membersihkan layar
				GUI_Clear();

				// menampilkan countdown

//				OldFont = GUI_SetFont(&GUI_Font8x16); // Buffer old font; mengubah font
				OldFont = GUI_SetFont(&GUI_FontD64); // Buffer old font; mengubah font

//				GUI_DispStringAt("This text is 8 by 16 pixels", 0, 0);
				for (int k = 10;k>0;k--)
				{
					GUI_DispDecAt(k,16,0,2); // Disp value
					vTaskDelay(pdMS_TO_TICKS(BLINKING_DELAY_MS)) ;
				}
			}
		}
}
