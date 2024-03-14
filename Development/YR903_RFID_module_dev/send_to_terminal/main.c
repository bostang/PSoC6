#include "cyhal.h"
//#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define DATA_BITS_8     8
#define STOP_BITS_1     1
#define BAUD_RATE       115200
#define UART_DELAY      10u
//#define RX_BUF_SIZE     4
//#define TX_BUF_SIZE     4
#define RX_BUF_SIZE     5
#define TX_BUF_SIZE     5

/* Variable Declarations */
cy_rslt_t rslt;
cyhal_uart_t uart_obj;
uint32_t actualbaud;
uint8_t tx_buf[TX_BUF_SIZE] = {0xA0, 0x03, 0x01, 0x70, 0xEC}; // cmd_reset byte array
uint8_t rx_buf[RX_BUF_SIZE];
size_t tx_length = TX_BUF_SIZE;
size_t rx_length = RX_BUF_SIZE;

/* Initialize the UART configuration structure */
const cyhal_uart_cfg_t uart_config =
{
    .data_bits = DATA_BITS_8,
    .stop_bits = STOP_BITS_1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = rx_buf,
    .rx_buffer_size = RX_BUF_SIZE
};

int main(void)
{
    /* Initialize the UART Block */
    rslt = cyhal_uart_init(&uart_obj, CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, NC,NC,NULL,&uart_config);

    /* Set the baud rate */
    rslt = cyhal_uart_set_baud(&uart_obj, BAUD_RATE, &actualbaud);

    /* Begin Tx Transfer */
    cyhal_uart_write(&uart_obj, (void*)tx_buf, &tx_length);
    cyhal_system_delay_ms(UART_DELAY);

    /* Begin Rx Transfer */
    cyhal_uart_read(&uart_obj, (void*)rx_buf, &rx_length);
    cyhal_system_delay_ms(UART_DELAY);

    /* Clean up and other application logic */
    // ...

    for (;;)
    {
        // Application loop
    }
}
///* [] END OF FILE */
