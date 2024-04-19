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

/*******************************************************************************
* Macros
********************************************************************************/
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_STACK_SIZE   (1024)
#define OLED_TASK_PRIORITY     (5)

#define OLED_HOR		(128)
#define OLED_VER		(64)

#define BLINKING_DELAY_MS		(1000)

const GUI_FONT * OldFont;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void oled_task(void* arg);


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
void oled_task(void* arg)
{
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

		GUI_DrawBitmap(&bmBMP_inventrix_logo,0,0);

		for(;;)
		{
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
