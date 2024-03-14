// 2023/03/14 malam jam 22.35
    // dibuat di luar infinite for loop
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define DATA_BITS_8     8
#define STOP_BITS_1     1
#define BAUD_RATE       115200
#define UART_DELAY      10u
#define RX_BUF_SIZE     1
#define TX_BUF_SIZE     6
//#define LEN_EPC			12 // 12 double
#define LEN_EPC			12 // 12 double

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
size_t epc_length = LEN_EPC;



// Variabel flag untuk menandakan telah terdeteksi tag
volatile int flag_tag_detect= 0;

// variabel flag untuk deteksi header (0x0A)
volatile int flag_header_detect= 0;

// variabel flag untuk menyatakan telah selesai deteksi EPC
volatile int flag_finish_read= 0;

// indeks tag
int index_tag = -7;

// variabel EPC
uint8_t epc[LEN_EPC];


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

    /* Initialize the UART Blocks */
    rslt = cyhal_uart_init(&uart_rfid, P12_1, P12_0, NC, NC, NULL, &uart_rfid_config);
    rslt = cyhal_uart_init(&uart_usb, P9_1, P9_0, NC, NC, NULL, &uart_usb_config);

    /* Set the baud rates */
    rslt = cyhal_uart_set_baud(&uart_rfid, BAUD_RATE, &actualbaud);
    rslt = cyhal_uart_set_baud(&uart_usb, BAUD_RATE, &actualbaud);

    /* Begin Tx Transfer */
    // Send the command to start reading from the RFID scanner
    cyhal_uart_write(&uart_rfid, (void*)tx_buf, &tx_length);

    /* Begin Rx Transfer */
    // Read the response data from the RFID scanner
    	// tunggu sampai data pertama terdeteksi
    while (!flag_tag_detect)
    {
    	if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_rfid, &rx_buf, 0))
    	{
    		flag_tag_detect = 1;
    	}
    		// tulis tag pertama
    	if (CY_RSLT_SUCCESS != cyhal_uart_write(&uart_usb, (void*)rx_buf, &rx_length))
		{
			handle_error();
		}

    	if (rx_buf[0] == 0xA0)
    	{
    		flag_header_detect = 1;
    	}
    	if (flag_header_detect)
    	{
    		if (index_tag > 0)
    		{
    			epc[index_tag] = rx_buf[0];
    		}
    		index_tag++;
    	}
    }

    // terus baca sampai akhir
    while((CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_rfid, &rx_buf, 0))&&(!flag_finish_read))
	{

		// tulis ke terminal
		if (CY_RSLT_SUCCESS != cyhal_uart_write(&uart_usb, (void*)rx_buf, &rx_length))
		{
			handle_error();
		}
    	if (rx_buf[0] == 0xA0)
    	{
    		flag_header_detect = 1;
    	}
    	if (flag_header_detect)
    	{
    		if (index_tag >= 0)
    		{
    			epc[index_tag] = rx_buf[0];
    		}
    		index_tag++;
    		if (index_tag > LEN_EPC)
    		{
    			flag_finish_read = 1;
    		}
    	}
	}

    // menampilkan tag ke usb-ttl
    for (size_t i = 0;i<LEN_EPC;i++)
    {
		cyhal_uart_putc(&cy_retarget_io_uart_obj,epc[i]);
    }

    for (;;)
    {

    }
}
///* [] END OF FILE */
