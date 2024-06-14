/*******************************************************************************
* File Name:   led.h (WMS v2.0)
* Description: implementasi task led
* Programmer: Bostang
* Tanggal : 2024/04/04
*******************************************************************************/

// HEADER GUARD
#ifndef LED_H
#define LED_H

/*******************************************************************************
* Macros
********************************************************************************/
#define RED_LED_TASK_NAME         			("Red LED Task")
#define GREEN_LED_TASK_NAME         		("Green LED Task")
#define LED_TASK_STACK_SIZE   				(1024)
#define LED_TASK_PRIORITY     				(1)

#define RED_BLINKY_LED_TASK_DELAY_TICKS  	(500)
#define GREEN_BLINKY_LED_TASK_DELAY_TICKS  	(200)

#define PIN_RED_LED 						(P9_4)
#define PIN_GREEN_LED 						(P9_2)

#define LED_ON								(1)
#define LED_OFF								(0)

UBaseType_t __attribute__((used)) uxTopUsedPriority;
#define uxTopReadyPriority 					(uxTopUsedPriority)

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void red_led_task(void *pvParameters);
void green_led_task(void *pvParameters);


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
void red_led_task(void *pvParameters)
{
	TaskParameters *params = (TaskParameters *)pvParameters;
	int* state = &(params->state);

    printf("%s started!\r\n", RED_LED_TASK_NAME);

    /* Initialize the User LED */
    cyhal_gpio_init(PIN_RED_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);

    for(;;)
    {
    	if ((*state == STATE_SCAN) || (*state == STATE_SEND))
        {
        	cyhal_gpio_write((cyhal_gpio_t)PIN_RED_LED, LED_ON);
        }
        else
        {
        	cyhal_gpio_write((cyhal_gpio_t)PIN_RED_LED, LED_OFF);
        }
        vTaskDelay(50);

    	if (true == gpio_intr_flag_mode)
		{
    		for (int k = 0;k<150;k++)
    		{
    			cyhal_gpio_toggle((cyhal_gpio_t)PIN_RED_LED);
				vTaskDelay(100);
    		}
		}
        taskYIELD(); // Yield to other tasks
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
void green_led_task(void *pvParameters)
{
	TaskParameters *params = (TaskParameters *)pvParameters;
	int* state = &(params->state);

    printf("%s started!\r\n", GREEN_LED_TASK_NAME);

    /* Initialize the User LED */
    cyhal_gpio_init(PIN_GREEN_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);

	cyhal_gpio_write((cyhal_gpio_t)PIN_GREEN_LED, LED_ON);


    for(;;)
    {

    	if ((*state == STATE_DISPLAY) || (*state == STATE_SEND))
        {
        	cyhal_gpio_write((cyhal_gpio_t)PIN_GREEN_LED, LED_ON);
        }
        else
        {
        	cyhal_gpio_write((cyhal_gpio_t)PIN_GREEN_LED, LED_OFF);
        }
        vTaskDelay(50);


    	if (true == gpio_intr_flag_mode)
		{
    		cyhal_gpio_write((cyhal_gpio_t)PIN_GREEN_LED, LED_OFF);
		}
	taskYIELD(); // Yield to other tasks
	}
}

#endif // LED_H

/* [] END OF FILE */
