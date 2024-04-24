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
* Global Variables
*******************************************************************************/
bool flag_epc_displayed = 0;// menyatakan apakah tag sudah ditampilkan pada OLED/belum

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

		// menampilkan logo inventrix di awal selama 2 detik
		GUI_DrawBitmap(&bmBMP_inventrix_logo,0,0);

		// delay 2 detik
		vTaskDelay(pdMS_TO_TICKS(2*BLINKING_DELAY_MS)) ;
		// membersihkan layar
		GUI_Clear();


		for(;;)
		{
//
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

			//    uint8_t variabel_kirim[13] = {'8','9','9','2','7','7','9','2','5','1','4','0','5'};

				//membuat string json forma
		}
}
