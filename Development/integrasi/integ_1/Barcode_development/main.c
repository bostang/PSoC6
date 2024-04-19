/******************************************************************************
* File Name:   main.c
*
* Description: Melakukan scanning dari barcode scanner lalu menampilkan pada terminal
*
* Programmer : Bostang Palaguna
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <stdlib.h>
	/* FreeRTOS header file. */
#include <FreeRTOS.h>
#include <task.h>
#include "barcode.h"

/*******************************************************************************
* Global Variables
********************************************************************************/
/* This enables RTOS aware debugging. */
volatile int uxTopUsedPriority;

volatile bool gpio_intr_flag = false;
cyhal_gpio_callback_data_t gpio_btn_callback_data;
bool state_toggle = false; // Initial state

/* Displaying task handle. */
TaskHandle_t barcode_taskHandle;




/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  uint32_t status - status indicates success or failure
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(uint32_t status)
{
    if (status != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

/*******************************************************************************
* Function Name: gpio_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
    gpio_intr_flag = true;
}


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function sets up user tasks and then starts
*  the RTOS scheduler.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* This enables RTOS aware debugging in OpenOCD. */
	uxTopUsedPriority = configMAX_PRIORITIES - 1;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* To avoid compiler warnings. */
	(void) result;


	/* Initialize the user button */
	    result = cyhal_gpio_init(BTN_PIN, CYHAL_GPIO_DIR_INPUT,
	    		CYHAL_GPIO_DRIVE_NONE , CYBSP_BTN_OFF);
	/* User button init failed. Stop program execution */
	handle_error(result);

	/* Configure GPIO interrupt */
	 gpio_btn_callback_data.callback = gpio_interrupt_handler;
	 cyhal_gpio_register_callback(BTN_PIN,
								  &gpio_btn_callback_data);
	 cyhal_gpio_enable_event(BTN_PIN, CYHAL_GPIO_IRQ_RISE,
								  GPIO_INTERRUPT_PRIORITY, true);

	// Initialize the UART configuration structure
	const cyhal_uart_cfg_t uart_config =
	{
		.data_bits      = 8,
		.stop_bits      = 1,
		.parity         = CYHAL_UART_PARITY_NONE,
		.rx_buffer      = rx_buf,
		.rx_buffer_size = RX_BUF_SIZE
	};

	// Initialize the UART Block
	cyhal_uart_init(&uart_obj, BAR_TX, BAR_RX, NC, NC, NULL,
						   &uart_config);

	// Set the baud rate
	cyhal_uart_set_baud(&uart_obj, 115200, &actualbaud);

	/* Enable global interrupts. */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port. */
	cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,CY_RETARGET_IO_BAUDRATE);

	/* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen. */
	printf("\x1b[2J\x1b[;H");
	printf("============================================================\r\n");
	printf("Unit Testing : Barcode Scanner\r\n");
	printf("============================================================\r\n");

	xTaskCreate(barcode_task, "Button task", BAR_TASK_STACK_SIZE, NULL, BAR_TASK_PRIORITY, &barcode_taskHandle);

	/* Start the FreeRTOS scheduler. */
	vTaskStartScheduler();

	/* Should never get here. */
	CY_ASSERT(0);
}
