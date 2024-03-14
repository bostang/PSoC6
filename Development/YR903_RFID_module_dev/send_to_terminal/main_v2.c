#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define DATA_BITS_8     8
#define STOP_BITS_1     1
#define BAUD_RATE       115200
#define TX_BUF_SIZE     6
#define RX_BUF_SIZE     1 // Read one character at a time

cyhal_uart_t uart_rfid; // UART object for RFID scanner
cyhal_uart_t uart_usb;  // UART object for USB TTL
uint8_t tx_buf[TX_BUF_SIZE] = {0xA0, 0x04, 0x01, 0x89, 0x01, 0xD1}; // cmd_read byte array
size_t tx_length = TX_BUF_SIZE;
uint8_t rx_buf[RX_BUF_SIZE];
uint8_t rx_buf2[RX_BUF_SIZE];
size_t rx_length = RX_BUF_SIZE;

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

/* Event handler callback function */
void uart_event_handler(void* handler_arg, cyhal_uart_event_t event)
{
    if (event == CYHAL_UART_IRQ_TX_DONE) {
        // TX done event
        // You can add any necessary code here
    }
    // Add more conditions for other events if needed
}

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

    // Initialize the device and board peripherals
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS) {
        // Handle initialization error
    }

    // Initialize retarget-io to use the debug UART port
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) {
        // Handle initialization error
    }

    // Initialize the UART Blocks
    result = cyhal_uart_init(&uart_rfid, P12_1, P12_0, NC, NC, NULL, &uart_rfid_config);
    if (result != CY_RSLT_SUCCESS) {
        // Handle initialization error
    }
    result = cyhal_uart_init(&uart_usb, P9_1, P9_0, NC, NC, NULL, &uart_usb_config);
    if (result != CY_RSLT_SUCCESS) {
        // Handle initialization error
    }

    // Set the baud rates
    result = cyhal_uart_set_baud(&uart_rfid, BAUD_RATE, NULL);
    if (result != CY_RSLT_SUCCESS) {
        // Handle initialization error
    }
    result = cyhal_uart_set_baud(&uart_usb, BAUD_RATE, NULL);
    if (result != CY_RSLT_SUCCESS) {
        // Handle initialization error
    }

    // Register UART event handler callback
    cyhal_uart_register_callback(&uart_rfid, uart_event_handler, NULL);

    // Enable UART RX_DONE event
    cyhal_uart_enable_event(&uart_rfid, CYHAL_UART_IRQ_RX_DONE, 3, true);

    // Begin TX transfer
    result = cyhal_uart_write(&uart_rfid, (void*)tx_buf, &tx_length);
    if (result != CY_RSLT_SUCCESS) {
        // Handle write error
    }

    // Main loop
    for (;;) {
        // Add any necessary code here
    }
}
