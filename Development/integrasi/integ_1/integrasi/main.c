/*******************************************************************************
* File Name:   main.c (Integrasi 1)
* Description: Melakukan pengujian integrasi antara sub-sistem data acquisition & processing dengan sistem informasi berupa state chart sederhana :
* 	(IDLE) --ACQ--> (SCAN) --ACQ--> (DISPLAY) --ACQ--> (SEND)
* Programmer: Bostang
* Tanggal : 2024/04/04
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "FreeRTOS.h"
#include "task.h"

// include file libary yang di-deklarasi-kan sendiri
#include "button.h"
#include "utils.h"
#include "led.h"
#include "fsm.h"
#include "oled.h"

/*******************************************************************************
* Macros
*******************************************************************************/

/*******************************************************************************
* Global Variables
*******************************************************************************/
cy_rslt_t result;

// inisiasi state saat ini dengan STATE_IDLE
State state = STATE_IDLE;


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
    /* INISIASI KOMPONEN, INTERRUPT, DAN TASK */
    initialize_buttons_and_interrupts();

    /* Enable global interrupts */
    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("**************** Integration Test 001 ****************\r\n");

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

//	/* Initialize the I2C to use with the OLED display */
//	// local variable untuk inisiasi I2C
//	cyhal_i2c_t i2c_obj;
//	volatile bool flag_i2c_init_success=false;
//	volatile bool flag_oled_init_success=false;
//	volatile bool flag_GUI_init_success=false;
//
//   while(!flag_i2c_init_success)
//   {
//		if (CY_RSLT_SUCCESS == cyhal_i2c_init(&i2c_obj, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL))
//		{
//			flag_i2c_init_success = true;
//			printf("i2c init sukses\r\n");
//		}
//   }
//
//   /* Initialize the OLED display */
//   while(!flag_oled_init_success)
//   {
//	   if (CY_RSLT_SUCCESS == mtb_ssd1306_init_i2c(&i2c_obj))
//		{
//			flag_oled_init_success = true;
//			printf("mtb ssd1306 init sukses\r\n");
//		}
//   }
//
//	while(!flag_GUI_init_success)
//	{
//		if (CY_RSLT_SUCCESS == GUI_Init())
//		{
//			flag_GUI_init_success = true;
//			printf("GUI init sukses\r\n");
//		}
//	}

	/* Inisiasi block I2C */
//	cyhal_i2c_t i2c_obj;
//	result = cyhal_i2c_init(&i2c_obj, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
//	if (result != CY_RSLT_SUCCESS)
//	{
//		printf("\r\nError: Inisiasi I2C gagal!\r\n");
//		CY_ASSERT(0);
//	}
//
//	/* Inisiasi library OLED display */
//	result = mtb_ssd1306_init_i2c(&i2c_obj);
//	if (result != CY_RSLT_SUCCESS)
//	{
//		printf("\r\nError: Inisiasi Library OLED gagal!\r\n");
//		CY_ASSERT(0);
//	}

//	/* Inisiasi Library emWin */
//	GUI_Init();

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
	retval = xTaskCreate(button_task_acq, "Button Task Acquire", BTN_TASK_STACK_SIZE, &state, BTN_TASK_PRIORITY, NULL);
	if (pdPASS != retval)
	{
		printf("Button Task Acquire creation failed!\r\n");
	}

	retval = xTaskCreate(button_task_prev, "Button Task Previous", BTN_TASK_STACK_SIZE, &state, BTN_TASK_PRIORITY, NULL);
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
	retval = xTaskCreate( red_led_task, RED_LED_TASK_NAME,LED_TASK_STACK_SIZE, &state, LED_TASK_PRIORITY, NULL );

	if( pdPASS != retval )
	{
		 printf("%s did not start!\r\n", RED_LED_TASK_NAME);
	}

	retval = xTaskCreate( green_led_task, GREEN_LED_TASK_NAME,LED_TASK_STACK_SIZE, &state, LED_TASK_PRIORITY, NULL );
	if (pdPASS != retval)
	{
		printf("Green LED Task Mode creation failed!\r\n");
	}

	/* Create task to handle oled events */
//	retval = xTaskCreate( oled_task, OLED_TASK_NAME,OLED_TASK_STACK_SIZE, &state, OLED_TASK_PRIORITY, NULL );
//	if (pdPASS != retval)
//	{
//		printf("OLED Task Mode creation failed!\r\n");
//	}

	printf("Scheduler start!\r\n");

	/* Start FreeRTOS scheduler */
	vTaskStartScheduler();
}
