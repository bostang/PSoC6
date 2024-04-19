/*******************************************************************************
* File Name:   oled.h (Integrasi 1)
* Description: implementasi task oled display (ssd1306)
* Programmer: Bostang
* Tanggal : 2024/04/07
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "fsm.h"
#include "cybsp.h"
#include "mtb_ssd1306.h"
#include "GUI.h"
#include "cyhal.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_STACK_SIZE   (1024)
#define OLED_TASK_PRIORITY     (5)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void oled_task(State* arg);


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
*  State* context passed from main function
*
* Return:
*  void
*
*******************************************************************************/
void oled_task(State* arg)
{
	cy_rslt result;

	/* Inisiasi block I2C */
	result = cyhal_i2c_init(&i2c_obj, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
	if (result != CY_RESULT_SUCCESS)
	{
		printf("\r\nError: Inisiasi I2C gagal!\r\n");
		CY_ASSERT(0);
	}

	/* Inisiasi library OLED display */
	result = mtb_ssd1306_init_i2c(&i2c_obj);
	if (result != CY_RESULT_SUCCESS)
	{
		printf("\r\nError: Inisiasi Library OLED gagal!\r\n");
		CY_ASSERT(0);
	}


	/* Inisiasi Library emWin */
	GUI_Init();

	GUI_DispString("Hello world!");

	for(;;)
	{
	}
}
