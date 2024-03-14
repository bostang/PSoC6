// 2023/03/14 pagi - siang jam 1
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define DATA_BITS_8     8
#define STOP_BITS_1     1
#define BAUD_RATE       115200
#define UART_DELAY      10u
//#define UART_DELAY      1300u
#define RX_BUF_SIZE     48
#define TX_BUF_SIZE     6

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}


/* Variable Declarations */
cy_rslt_t rslt;
cyhal_uart_t uart_rfid; // UART object for RFID scanner
cyhal_uart_t uart_usb;  // UART object for USB TTL
uint32_t actualbaud;
uint8_t tx_buf[TX_BUF_SIZE] = {0xA0, 0x04, 0x01, 0x89, 0x01, 0xD1}; // cmd_read byte array
uint8_t rx_buf[RX_BUF_SIZE];
uint8_t rx_buf2[RX_BUF_SIZE];
size_t tx_length = TX_BUF_SIZE;
size_t rx_length = RX_BUF_SIZE;

/* Initialize the UART configuration structures */
const cyhal_uart_cfg_t uart_rfid_config =
{
    .data_bits = DATA_BITS_8,
    .stop_bits = STOP_BITS_1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = rx_buf,
    .rx_buffer_size = RX_BUF_SIZE
};

const cyhal_uart_cfg_t uart_usb_config =
{
    .data_bits = DATA_BITS_8,
    .stop_bits = STOP_BITS_1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = rx_buf2, // Use the same buffer for both UARTs or allocate separate buffers
    .rx_buffer_size = RX_BUF_SIZE
};


int main(void)
{
	 cy_rslt_t result;
	    #if defined(CY_DEVICE_SECURE)
	        cyhal_wdt_t wdt_obj;
	        /* Clear watchdog timer so that it doesn't trigger a reset */
	        result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
	        CY_ASSERT(CY_RSLT_SUCCESS == result);
	        cyhal_wdt_free(&wdt_obj);
	    #endif

	    uint8_t read_data; /* Variable to store the received character
	                        * through terminal */

	    /* Initialize the device and board peripherals */
	    result = cybsp_init();
	    if (result != CY_RSLT_SUCCESS)
	    {
	        handle_error();
	    }

	    /* Initialize retarget-io to use the debug UART port */

	    result = cy_retarget_io_init_fc(CYBSP_DEBUG_UART_TX,
	                                    CYBSP_DEBUG_UART_RX,
	                                    CYBSP_DEBUG_UART_CTS,
	                                    CYBSP_DEBUG_UART_RTS,
	                                    CY_RETARGET_IO_BAUDRATE);

	    if (result != CY_RSLT_SUCCESS)
	    {
	        handle_error();
	    }

	    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
	    printf("\x1b[2J\x1b[;H");

	    printf("***********************************************************\r\n");

    /* Initialize the UART Blocks */
    rslt = cyhal_uart_init(&uart_rfid, P12_1, P12_0, NC, NC, NULL, &uart_rfid_config);
    rslt = cyhal_uart_init(&uart_usb, P9_1, P9_0, NC, NC, NULL, &uart_usb_config);

    /* Set the baud rates */
    rslt = cyhal_uart_set_baud(&uart_rfid, BAUD_RATE, &actualbaud);
    rslt = cyhal_uart_set_baud(&uart_usb, BAUD_RATE, &actualbaud);

    /* Begin Tx Transfer */
    // Send the command to start reading from the RFID scanner
    cyhal_uart_write(&uart_rfid, (void*)tx_buf, &tx_length);
//    cyhal_system_delay_ms(UART_DELAY);


    /* Begin Rx Transfer */
    // Read the response data from the RFID scanner
    cyhal_uart_read(&uart_rfid, (void*)rx_buf, &rx_length);
    cyhal_system_delay_ms(UART_DELAY);
//    printf("")
//
    // Print the received data from the RFID reader
    printf("Received data from RFID reader: \r\n");
    for (size_t i = 0; i < rx_length; i++) {
        printf("%02X\r\n", rx_buf[i]); // Assuming you want to print in hexadecimal format
    }
    printf("\r\n");
//
//    // Send the read data to the USB-TTL
//    cyhal_uart_write(&uart_usb, (void*)rx_buf, &rx_length);
//    cyhal_system_delay_ms(UART_DELAY);
//
    // ...

    for (;;)
    {
        // Application loop
    }
}
///* [] END OF FILE */
