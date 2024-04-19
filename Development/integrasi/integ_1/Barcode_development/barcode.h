/*******************************************************************************
* File Name:   barcode.h
* Description: implementasi Barcode sebagai bagian dari sub-sistem data acquisition
* 				apabila button ditekan, maka akan melakukan uart scanning (menunggu karakter input dari barcode scanner dan kemudian menampilkan bacaan pada terminal
* 				dan juga OLED
* Programmer: Bostang
* Tanggal awal : 18 April 2024
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // For malloc and free
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define BAR_RX P9_0
#define BAR_TX P9_1

/*******************************************************************************
* Macros
********************************************************************************/
/* Button related macros */
#define DEBOUNCING_DELAY_MS          (1000)   /* milliseconds */
#define GPIO_INTERRUPT_PRIORITY (7u)
#define BTN_PIN P13_4

#define BAR_TASK_NAME         ("Button Task")
#define BAR_TASK_STACK_SIZE   (1024)
#define BAR_TASK_PRIORITY     (5)

/* RTOS related macros. */
#define displaying_task_STACK_SIZE        (5 * 1024)
#define displaying_task_PRIORITY          (1)

/* UART related macros. */
#define RX_BUF_SIZE     4
#define TX_BUF_SIZE     13
size_t tx_length = TX_BUF_SIZE;


/*******************************************************************************
* Global Variables
********************************************************************************/
int count_pressed = 0;
uint8_t read_data; /* Variable to store the received character
						* through terminal */
cyhal_uart_t uart_obj;
uint32_t     actualbaud;
uint8_t tx_buf[TX_BUF_SIZE]; // untuk ditampilkan ke terminal
uint8_t rx_buf[RX_BUF_SIZE];

/*******************************************************************************
* Function Prototypes
********************************************************************************/
uint8_t* data_acquisition_task();
void displaying_task();
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);
void barcode_task( void * arg );

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: barcode_task
********************************************************************************
* Summary:
*   Task to handle button events.
*
* Parameters:
*   void *arg: Task arguments (unused)
*
*******************************************************************************/
void barcode_task( void * arg )
{
    (void)arg;

    printf("%s started!\r\n", BAR_TASK_NAME);
    for(;;)
    {
 /* Check the interrupt status */
    	if (gpio_intr_flag)
		{
    		count_pressed++;
			gpio_intr_flag = false;
			printf("Button Pressed (x%d)\r\n",count_pressed);
			displaying_task();
			// alur pemanggilan : button task -> displaying task -> data acquisition task
		}
        vTaskDelay(DEBOUNCING_DELAY_MS); // Delay untuk debouncing
    }
}

/*******************************************************************************
 * Function Name: displaying_task
 *******************************************************************************
 * Summary:
 *  Memanggil data_acquisition_task lalu menampilkan hasilnya pada terminal
 *
 * Return:
 *  none
 *
 *******************************************************************************/
void displaying_task()
{
	printf("Calling : displaying task.\r\n");
	uint8_t* resultArray = malloc(TX_BUF_SIZE * sizeof(uint8_t)); // sebagai return value dari data_acquisition_task

	resultArray = data_acquisition_task();
	if (resultArray != NULL)
	{
	   // Copy the contents of the array to tx_buf
	   memcpy(tx_buf, resultArray, TX_BUF_SIZE * sizeof(uint8_t));

	   // Don't forget to free the memory allocated by the function
	   free(resultArray);
	}
	// Menuliskan tx_buf ke terminal
   cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)tx_buf, &tx_length);
   printf("\r\n\r\n");
}

/*******************************************************************************
 * Function Name: data_acquisition_task
 *******************************************************************************
 * Summary:
 *  Task used to retrieve data from barcode scanner
 *
 * Return:
 *  array of character (uint8_t)
 *
 *******************************************************************************/
uint8_t* data_acquisition_task()
{
	printf("Calling : data_acquisition task.\r\n");

    // inisiasi variabel data_barcode sebagai return value
    uint8_t* data_barcode = malloc(TX_BUF_SIZE * sizeof(uint8_t));

    // Handle memory allocation failure
    if (data_barcode == NULL)
    {
        return NULL;
    }

    // membaca karakter barcode sebesar TX_BUF_SIZE dan menyimpannya ke array data_barcode
    for (size_t i = 0;i<TX_BUF_SIZE;i++)
    {
    	if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_obj, &read_data, 0))
		{
			data_barcode[i]=read_data;
		}
    }

    return data_barcode;
}

/* [] END OF FILE */
