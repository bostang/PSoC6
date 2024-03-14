#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define DATA_BITS_8     8
#define STOP_BITS_1     1
#define BAUD_RATE       115200
#define UART_DELAY      10u
#define RX_BUF_SIZE     1
#define TX_BUF_SIZE     6
#define LEN_EPC         12 // 12 double

cyhal_uart_t uart_rfid; // UART object for RFID scanner
cyhal_uart_t uart_usb;  // UART object for USB TTL
uint8_t tx_buf[TX_BUF_SIZE] = {0xA0, 0x04, 0x01, 0x89, 0x01, 0xD1}; // cmd_read byte array
uint8_t rx_buf[RX_BUF_SIZE];
uint8_t rx_buf2[RX_BUF_SIZE];
size_t tx_length = TX_BUF_SIZE;
size_t rx_length = RX_BUF_SIZE;
size_t epc_length = LEN_EPC;
volatile int flag_tag_detect = 0;
int index_tag = -7;
uint8_t epc[LEN_EPC];

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

// Function to handle data reception from RFID scanner
void handle_rfid_data(void *callback_arg, cyhal_uart_event_t event)
{
    // Check if data received event
    if (event & CYHAL_UART_IRQ_RX_NOT_EMPTY)
    {
        cyhal_uart_read(&uart_rfid, (void*)rx_buf, &rx_length);
        // Process received data here
        if (rx_buf[0] == 0xA0)
        {
            flag_tag_detect = 1;
        }
        if (flag_tag_detect)
        {
            if ((index_tag >= 0) && (index_tag < epc_length))
            {
                epc[index_tag] = rx_buf[0];
                index_tag++;
            }
            if (index_tag == epc_length)
            {
                for (int i = 0; i < epc_length; i++)
                {
                    printf("%x", epc[i]);
                }
                printf("\r\n");
            }
        }
        cyhal_uart_write(&uart_usb, (void*)rx_buf, &rx_length);
    }
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

    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    result = cy_retarget_io_init_fc(CYBSP_DEBUG_UART_TX,
                                    CYBSP_DEBUG_UART_RX,
                                    CYBSP_DEBUG_UART_CTS,
                                    CYBSP_DEBUG_UART_RTS,
                                    CY_RETARGET_IO_BAUDRATE);

    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    result = cyhal_uart_init(&uart_rfid, P12_1, P12_0, NC, NC, NULL, &uart_rfid_config);
    result = cyhal_uart_init(&uart_usb, P9_1, P9_0, NC, NC, NULL, &uart_usb_config);

    result = cyhal_uart_set_baud(&uart_rfid, BAUD_RATE, NULL);
    result = cyhal_uart_set_baud(&uart_usb, BAUD_RATE, NULL);

    // Register the callback for RFID UART
    cyhal_uart_register_callback(&uart_rfid, handle_rfid_data, NULL);
    // Enable the RX interrupt
    cyhal_uart_enable_event(&uart_rfid, CYHAL_UART_IRQ_RX_NOT_EMPTY, CYHAL_ISR_PRIORITY_DEFAULT, true);

    cyhal_uart_write(&uart_rfid, (void *)tx_buf, &tx_length);

    for (;;)
    {
        // Your main loop code here
    }
}
