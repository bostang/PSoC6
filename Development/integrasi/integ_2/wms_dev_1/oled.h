/*******************************************************************************
* File Name:   oled.h (Integrasi 6)
* Description: implementasi task oled display (ssd1306)
* Programmer: Bostang
* Tanggal : 2024/04/07
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cybsp.h"
#include "mtb_ssd1306.h"
#include "GUI.h"
#include "cyhal.h"

#include "utils.h"
#include "oled_img.h"

#include "fsm.h"

#include "cyabs_rtos.h"

#include "data_acquisition.h"

#include "utils.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_STACK_SIZE   (1024)
#define OLED_TASK_PRIORITY     (5)
//#define OLED_TASK_PRIORITY     (4)		// diturunkan prioritasnya untuk dilihat apakah akan mengalah masalah mutex dengan rfid_task

#define OLED_HOR		(128)
#define OLED_VER		(64)

#define BLINKING_DELAY_MS		(1000)

const GUI_FONT * OldFont;

/*******************************************************************************
* Global Variables
*******************************************************************************/
bool flag_epc_displayed = false;// menyatakan apakah tag sudah ditampilkan pada OLED/belum
bool flag_oled_display = false; // agar state hanya tampil sekali -> di setiap state, kondisi pemeriksaan bergantian (if !flag.., if flag..)
bool flag_epc_scanned = false; //menyatakan apakah sudah ada epc yang terbaca atau belum

uint8_t* resultArray;


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

		// menampilkan logo inventrix di awal selama 2 detik
		GUI_DrawBitmap(&bmBMP_inventrix_logo,0,0);

		// delay 2 detik
		vTaskDelay(pdMS_TO_TICKS(2*BLINKING_DELAY_MS)) ;
		// membersihkan layar
		GUI_Clear();


		for(;;)
		{

				if (*state == STATE_IDLE)
				{
					if (!flag_oled_display)
					{
						GUI_GotoX(4);
						GUI_DispString("State IDLE\n");
						flag_oled_display = true;
					}
				}

				else if (*state == STATE_SCAN)
				{
					// menampilkan pada OLED
					if (flag_oled_display) // selang-seling
					{
						GUI_GotoX(4);
						GUI_DispString("State SCAN\n");
						flag_oled_display = false;
						*state = STATE_DISPLAY; // langsung pindah ke state display
					}
					// melakukan scanning dengan barcode
					if (!flag_epc_scanned)
					{

						size_t tx_length = TX_BUF_SIZE;
						resultArray = malloc(TX_BUF_SIZE * sizeof(uint8_t));

						// melakukan scanning dengan barcode
						resultArray = data_acquisition_task();
						cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)resultArray, &tx_length);
						printf("\r\nEPC terdeteksi!\r\n");
						flag_epc_scanned = true;
					}
				}
				else if (*state == STATE_DISPLAY)
				{
					if (!flag_oled_display)
					{
						GUI_GotoX(4);
						GUI_DispString("State DISPLAY\n");
						flag_oled_display = true;
					}

					// mengubah tag dari byte array menjadi array of string
//					 int arrayLength = sizeof(params->epc) / sizeof(params->epc[0]); // Get the length of the array
					 int arrayLength = sizeof(resultArray) / sizeof(resultArray[0]); // Get the length of the array

					// Convert byte array to hexadecimal string array
//						char** hexStringArray = byteArrayToHexStringArray(&resultArray, arrayLength);
//					 char** hexStringArray = byteArrayToHexStringArray(resultArray, arrayLength);
//					 char* hexStringArray = uint8ArrayToHexString(resultArray, arrayLength);
//					    free(resultArray); // mebebaskan resultArray setelah dikonversi menjadi array of string

//					 for (int k = 0;k<arrayLength;k++)
//					 {
//						 printf("%c",resultArray[k]);
//					 }
//					 printf("\r\n");

					 // Convert to string
					    char* stringBuffer = (char*)malloc(TX_BUF_SIZE + 1); // +1 for null-terminator
					    if (stringBuffer == NULL)
					    {
					        printf("Failed to allocate memory.\n");
					        return 1; // Return error code or handle allocation failure
					    }

					    // Copy each character from tx_buf to stringBuffer
					    for (size_t i = 0; i < TX_BUF_SIZE; i++) {
					        stringBuffer[i] = (char)resultArray[i];
					    }
					    stringBuffer[TX_BUF_SIZE] = '\0'; // Null-terminate the string

					    for (int i = 0;i<TX_BUF_SIZE;i++)
					    {
					    	printf("%c",stringBuffer[i]);
					    }
					    printf("\r\n");




					    // DEBUG : tampilkan array ke layar:
//					    printf("%s\r\n",hexStringArray);
//					    for (int i = 0; i < arrayLength; i++)
//					    {
//					        printf("%s", hexStringArray[i]);
//					    }
//					    printf("\r\n");

						// menampilkan epc hasil scan rfid
						if (!flag_epc_displayed) // menampilkan epc sekali
						{
							GUI_GotoX(4);
//							GUI_DispString(hexStringArray);

							for (int k = 0;k<TX_BUF_SIZE;k++)
							 {
//								 printf("%c",resultArray[k]);
//								GUI_DispString(hexStringArray);
								GUI_DispChar(stringBuffer[k]);

							 }
//							 printf("\r\n");
//							GUI_DispString("\n");

//							GUI_DispString(hexStringArray);
//							for (int k = 0; k<arrayLength; k++)
//							{
//								GUI_DispString(hexStringArray[k]);
//								GUI_DispString(" ");
//							}
							GUI_DispString("\n");

//						for (int k = 0;k<len_epc;k++)
//						{
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
//						GUI_DispString("\n");
						flag_epc_displayed = true;
						*state = STATE_SEND; // langsung pindah ke state display
					}
				}
				else if (*state == STATE_SEND)
				{
					if (flag_oled_display)
					{
						GUI_GotoX(4);
						GUI_DispString("State SEND\n");
						flag_oled_display = false;
					}
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
