/******************************************************************************
* File Name:   main.c (Demo 2)
*
* Description: Sebuah demo untuk melakukan scanning dari barcode kemudian menampilkan hasilnya pada OLED dan mengirimkan pada backend.
*
* Tanggal awal : 24 April 2024
******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/* FreeRTOS header file. */
#include <FreeRTOS.h>
#include <task.h>

/* user-defined task header file. */
#include "display.h"
#include "data_acquisition.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* RTOS related macros. */
//#define HTTP_CLIENT_TASK_STACK_SIZE        (5 * 1024)
#define HTTP_CLIENT_TASK_PRIORITY          (1)


/*******************************************************************************
* Global Variables
********************************************************************************/
/* This enables RTOS aware debugging. */
volatile int uxTopUsedPriority;

/* HTTP Client task handle. */
//TaskHandle_t client_task_handle;
TaskHandle_t display_task_handle;

TaskHandle_t data_acquisition_task_handle;


// uint8_t tx_buf[TX_BUF_SIZE];
// size_t tx_length = TX_BUF_SIZE;
// uint8_t variabel_kirim[TX_BUF_SIZE];

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

	/* Enable global interrupts. */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port. */
	cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,CY_RETARGET_IO_BAUDRATE);

	/* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen. */
	printf("\x1b[2J\x1b[;H");
	printf("============================================================\n");
	printf("Demo 2.0: SCAN RFID and display to OLED\n");
	printf("============================================================\n\n");

	/* Create the task. */
//	xTaskCreate(http_client_task, "Network task", HTTP_CLIENT_TASK_STACK_SIZE, NULL, HTTP_CLIENT_TASK_PRIORITY, &client_task_handle);
	xTaskCreate(display_task, "Display task", OLED_TASK_STACK_SIZE, NULL, OLED_TASK_PRIORITY, &display_task_handle);

	/* Start the FreeRTOS scheduler. */
	vTaskStartScheduler();

	/* Should never get here. */
	CY_ASSERT(0);
}

/* [] END OF FILE */
