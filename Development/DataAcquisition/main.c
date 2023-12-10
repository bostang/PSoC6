// Program DataAcquisition
	// Realisasi modul data acquisition pada subsistem data acquisition&processing warehouse management system
	// Yang menerima input data berupa tag RFID atau tag barcode
// KAMUS
	// Konstanta
		//
	// Fungsi / Prosedur
		// procedure handle_error
			// error handling function
	// Variabel
		//
// PINOUT
	// PSoC			<->		RFID Interface
		// ... [belum]
	// PSoC			<->		Barcode Scanner
		//	P9_1 (tx)	<->		Rx
		// 	P9_0 (rx)	<->		Tx
		//	5V			<->		5V
		//	GND			<->		GND

// Programmer : Bostang 

// ALGORITMA UTAMA

// INCLUDE LIBRARY
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

// DEKLARASI KONSTANTA
//#define pin_barcode_tx P9_1
//#define pin_barcode_rx P9_0
#define pin_barcode_tx P12_1
#define pin_barcode_rx P12_0
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

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function.
* Reads one byte from the serial terminal and echoes back the read byte.
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
#ifdef XMC72_EVK
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);

    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
#else
    cyhal_uart_t uart_obj;
    cyhal_uart_t uart_obj_barcode;
	#define TX_BUF_SIZE     4
    size_t tx_length = TX_BUF_SIZE;

   // ketika ini diaktifkan, maka kita bisa mengirim ke terminal dengan 'printf', tetapi akan konflik dengan cyhal_uart_init
//    result = cy_retarget_io_init_fc(CYBSP_DEBUG_UART_TX,
//    								CYBSP_DEBUG_UART_RX,
//                                    CYBSP_DEBUG_UART_CTS,
//                                    CYBSP_DEBUG_UART_RTS,
//                                    CY_RETARGET_IO_BAUDRATE);


    /* Initialize the UART configuration structure */
//    const cyhal_uart_cfg_t uart_config =
//    {
//        .data_bits = DATA_BITS_8,
//        .stop_bits = STOP_BITS_1,
//        .parity = CYHAL_UART_PARITY_NONE,
//        .rx_buffer = rx_buf,
//        .rx_buffer_size = RX_BUF_SIZE
//    };

    // mendeklarasikan uart object untuk USB-TTL (simulasi receiver)
    result = cyhal_uart_init(&uart_obj, CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,NC,NC, NULL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }


    // mendeklarasikan uart object untuk barcode reader (sender)
    result = cyhal_uart_init(&uart_obj_barcode, pin_barcode_tx, pin_barcode_rx,NC,NC, NULL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
#endif
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
//    printf("\x1b[2J\x1b[;H");

    // printf() menjadi tidak bisa dipakai karena retarget_io_init tidak dipakai
//    printf("***********************************************************\r\n");
//    printf("HAL: UART Transmit and Receive\r\n");
//    printf("***********************************************************\r\n\n");
//    printf(">> Start typing to see the echo on the screen \r\n\n");

    uint8_t tx_buf[TX_BUF_SIZE] = {'1','2','3','4'};
    char msg1[15] = "----------";
    size_t len_msg1 = sizeof(msg1);

    cyhal_uart_write(&uart_obj,(void*) tx_buf,&tx_length);
    cyhal_uart_putc(&uart_obj,'\n');
    cyhal_uart_write(&uart_obj,(void*) msg1,&len_msg1);
    cyhal_uart_putc(&uart_obj,'\n');

    __enable_irq();
 
    for (;;)
    {
        if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_obj_barcode,
                                               &read_data, 0))
        {
            if (CY_RSLT_SUCCESS != cyhal_uart_putc(&uart_obj,
                                                   read_data))
            {
                handle_error();
            }
        }
        else
        {
            handle_error();
        }
    }
}

/* [] END OF FILE */
