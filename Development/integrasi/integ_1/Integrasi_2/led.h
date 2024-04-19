/*******************************************************************************
* File Name:   led.h (Integrasi 1)
* Description: implementasi task led
* Programmer: Bostang
* Tanggal : 2024/04/04
*******************************************************************************/

/*******************************************************************************
* Macros
********************************************************************************/
#define RED_LED_TASK_NAME         ("Red LED Task")
#define GREEN_LED_TASK_NAME         ("Green LED Task")
#define LED_TASK_STACK_SIZE   (1024)
#define LED_TASK_PRIORITY     (5)

#define RED_BLINKY_LED_TASK_DELAY_TICKS  (500)
#define GREEN_BLINKY_LED_TASK_DELAY_TICKS  (200)

UBaseType_t __attribute__((used)) uxTopUsedPriority;
#define uxTopReadyPriority uxTopUsedPriority

#define PIN_RED_LED P9_4
#define PIN_GREEN_LED P9_2

#define LED_ON		(1)
#define LED_OFF		(0)

/*******************************************************************************
* Header Files
*******************************************************************************/
//#include "fsm.h"
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void red_led_task(void* arg);
void green_led_task(void* arg);

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Nyala LED :
*
* STATE		RED_LED		GREEN_LED
* ------------------------------------
* IDLE			0			0
* SCAN			1			0
* DISPLAY		0			1
* SEND			1			1
*******************************************************************************/

/*******************************************************************************
* Function Name: red_led_task
********************************************************************************
* Summary:
* Mendeskripsikan perilaku LED merah berdasarkan FSM
*
* Parameters:
*  void * context passed from main function
*
* Return:
*  void
*
*******************************************************************************/
void red_led_task(void* arg)
{
    printf("%s started!\r\n", RED_LED_TASK_NAME);

    /* Initialize the User LED */
    cyhal_gpio_init(PIN_RED_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);

    for(;;)
    {
//    	if ((*arg == STATE_SCAN) || (*arg == STATE_SEND))
//        {
//        	cyhal_gpio_write((cyhal_gpio_t)PIN_RED_LED, LED_ON);
//        }
//        else
//        {
//        	cyhal_gpio_write((cyhal_gpio_t)PIN_RED_LED, LED_OFF);
//        }
//        vTaskDelay(50);

    	if (true == gpio_intr_flag_mode)
		{
    		for (int k = 0;k<150;k++)
    		{
    			cyhal_gpio_toggle((cyhal_gpio_t)PIN_RED_LED);
				vTaskDelay(100);
    		}
		}
    }
}

/*******************************************************************************
* Function Name: green_led_task
********************************************************************************
* Summary:
* Mendeskripsikan perilaku LED hijau berdasarkan FSM
*
* Parameters:
*  void * context passed from main function
*
* Return:
*  void
*
*******************************************************************************/
void green_led_task(void* arg)
{
    (void)arg;

    printf("%s started!\r\n", GREEN_LED_TASK_NAME);

    /* Initialize the User LED */
    cyhal_gpio_init(PIN_GREEN_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);

	cyhal_gpio_write((cyhal_gpio_t)PIN_GREEN_LED, LED_ON);


    for(;;)
    {
//    	if ((*arg == STATE_DISPLAY) || (*arg == STATE_SEND))
//		{
//			cyhal_gpio_write((cyhal_gpio_t)PIN_GREEN_LED, LED_ON);
//		}
//		else
//		{
//			cyhal_gpio_write((cyhal_gpio_t)PIN_GREEN_LED, LED_OFF);
//		}
//		cyhal_gpio_toggle((cyhal_gpio_t)PIN_GREEN_LED);
//		vTaskDelay(1000);
    	if (true == gpio_intr_flag_mode)
		{
    		cyhal_gpio_write((cyhal_gpio_t)PIN_GREEN_LED, LED_OFF);
		}
	}
}

/* [] END OF FILE */
