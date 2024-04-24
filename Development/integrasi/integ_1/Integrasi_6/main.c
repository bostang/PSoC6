/*******************************************************************************
* File Name:   main.c (Integrasi 4)
* Description: Melakukan pengujian integrasi antara sub-sistem : Button ditekan, maka akan menjalankan barcode scanner
* 				disertai dengan bunyi buzzer dan LED
* Programmer: Bostang
* Tanggal : 2024/04/19
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cyabs_rtos.h"

// include file libary yang di-deklarasi-kan sendiri
#include "button.h"
#include "utils.h"
#include "led.h"
#include "oled.h"
#include "buzzer.h"

#include "mtb_ssd1306.h"
#include "GUI.h"

#include "barcode.h"
#include "rfid.h"

#include "fsm.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/
cy_rslt_t result;
TaskHandle_t barcode_taskHandle;
int state;
//TaskParameters taskParams;
TaskParameters taskParams =
{
   .state = 0,
   .epc= {"00","01","02","03","04","05","06","07","08","09","10","11"}
};

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void initialize_buttons_and_interrupts();
void tasks_creation();

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  Entry function for the application.
*  This function configures and initializes the GPIO
*  interrupt.
*
* Return: int
*
*******************************************************************************/
int main(void)
{
    // Initialize the mutex
    cy_rslt_t result = cy_rtos_mutex_init(&epc_mutex, false);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("Mutex initialization failed.\n");
        return -1;
    }

    /* INISIASI KOMPONEN, INTERRUPT, DAN TASK */
    initialize_buttons_and_interrupts();

    /* Enable global interrupts */
    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("**************** Integration Test 001 ****************\r\n");

    // Inisiasi State = state IDLE dan epc = 00 .. 00
//    state = STATE_IDLE;
//
    /* MEMBUAT TASK DAN MEMULAI SCHEDULER */
    tasks_creation();

    for (;;)
    {
        /* Code should not reach here */
    }
}

/*******************************************************************************
* Function Name: initialize_buttons_and_interrupts()
********************************************************************************
* Summary: Prosedur untuk inisiasi button dan interruptnya
*
* Return: -
*
*******************************************************************************/
void initialize_buttons_and_interrupts()
{
	/* Initialize the device and board peripherals */
	result = cybsp_init();
	/* Board init failed. Stop program execution */
	handle_error(result);

	/* Initialize retarget-io to use the debug UART port */
	result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
								 CY_RETARGET_IO_BAUDRATE);
	/* Retarget-io init failed. Stop program execution */
	handle_error(result);

	/* Initialize the user buttons */
	// button acquire
	result = cyhal_gpio_init(BTN_ACQ, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
	/* button init failed. Stop program execution */
	handle_error(result);

	// button previous
	result = cyhal_gpio_init(BTN_PREV, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
	handle_error(result);

	// button mode
	result = cyhal_gpio_init(BTN_MODE, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
	handle_error(result);

	/* Configure GPIO interrupts */
	gpio_btn_acq_callback_data.callback = gpio_interrupt_handler_acq;
	cyhal_gpio_register_callback(BTN_ACQ,&gpio_btn_acq_callback_data);
	cyhal_gpio_enable_event(BTN_ACQ, CYHAL_GPIO_IRQ_RISE, GPIO_INTERRUPT_PRIORITY, true);

	gpio_btn_prev_callback_data.callback = gpio_interrupt_handler_prev;
	cyhal_gpio_register_callback(BTN_PREV, &gpio_btn_prev_callback_data);
	cyhal_gpio_enable_event(BTN_PREV, CYHAL_GPIO_IRQ_RISE, GPIO_INTERRUPT_PRIORITY, true);

	gpio_btn_mode_callback_data.callback = gpio_interrupt_handler_mode;
	cyhal_gpio_register_callback(BTN_MODE,&gpio_btn_mode_callback_data);
	cyhal_gpio_enable_event(BTN_MODE, CYHAL_GPIO_IRQ_RISE, GPIO_INTERRUPT_PRIORITY, true);

	/* Inisiasi OLED : dilakukan di dalam tasknya */

	/* Inisiasi Buzzer */
    cyhal_gpio_init(BUZZ_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0U);

}

/*******************************************************************************
* Function Name: task_creation()
********************************************************************************
* Summary: Prosedur untuk membuat para task
*
* Return: -
*
*******************************************************************************/
void tasks_creation()
{
    int retval;

    /* Create tasks to handle button events */
	retval = xTaskCreate(button_task_acq, "Button Task Acquire", BTN_TASK_STACK_SIZE, (void *)&taskParams, BTN_TASK_PRIORITY, NULL);
	if (pdPASS != retval)
	{
		printf("Button Task Acquire creation failed!\r\n");
	}

	retval = xTaskCreate(button_task_prev, "Button Task Previous", BTN_TASK_STACK_SIZE, NULL, BTN_TASK_PRIORITY, NULL);
	if (pdPASS != retval)
	{
		printf("Button Task Previous creation failed!\r\n");
	}

	retval = xTaskCreate(button_task_mode, "Button Task Mode", BTN_TASK_STACK_SIZE, NULL, BTN_TASK_PRIORITY, NULL);
	if (pdPASS != retval)
	{
		printf("Button Task Mode creation failed!\r\n");
	}

	/* Create task to handle led events */
	retval = xTaskCreate( red_led_task, RED_LED_TASK_NAME,LED_TASK_STACK_SIZE, (void *)&taskParams, LED_TASK_PRIORITY, NULL );

	if( pdPASS != retval )
	{
		 printf("%s did not start!\r\n", RED_LED_TASK_NAME);
	}

	retval = xTaskCreate( green_led_task, GREEN_LED_TASK_NAME,LED_TASK_STACK_SIZE, (void *)&taskParams , LED_TASK_PRIORITY, NULL );
	if (pdPASS != retval)
	{
		printf("Green LED Task Mode creation failed!\r\n");
	}

	/* Create task to handle oled events */
	retval = xTaskCreate( oled_task, OLED_TASK_NAME,OLED_TASK_STACK_SIZE, (void *)&taskParams, OLED_TASK_PRIORITY, NULL );
	if (pdPASS != retval)
	{
		printf("OLED Task Mode creation failed!\r\n");
	}

	/* Create task to handle buzzer events */
	retval = xTaskCreate( buzzer_task, BUZZ_TASK_NAME,BUZZ_TASK_STACK_SIZE, NULL, BUZZ_TASK_PRIORITY, NULL );
	if (pdPASS != retval)
	{
		printf("Buzzer Task creation failed!\r\n");
	}

//	/* Create task to handle barcode events */
//	retval = 	xTaskCreate(barcode_task, "Button task", BAR_TASK_STACK_SIZE, NULL, BAR_TASK_PRIORITY, &barcode_taskHandle);
//	if (pdPASS != retval)
//	{
//		printf("Barcode Task creation failed!\r\n");
//	}

//	/* Create task to handle rfid scanner events */
//	retval = xTaskCreate( rfid_task, RFID_TASK_NAME,RFID_TASK_STACK_SIZE, (void *)&taskParams, RFID_TASK_PRIORITY, NULL );
//	if (pdPASS != retval)
//	{
//		printf("RFID Task creation failed!\r\n");
//	}

	printf("Scheduler start!\r\n");

	/* Start FreeRTOS scheduler */
	vTaskStartScheduler();
}
