// Program RFID
  // melakukan pembacaan RFID tag dan menampilkan pada serial monitor
  // Setiap button ditekan, maka satu kali siklus dijalankan
  // Saat LED hijau menyala, maka siap membaca RFID tag
  // Saat LED merah mati, maka RFID tag telah ditampilkan pada serial monitor
//pin komunikasi Serial2 untuk komunikasi dengan modul RFID
// KAMUS
  // Konstanta
    // maxTag := 100 { jumlah maksimum tag yang terbaca }
    // maxRespByte := 30
    // lenEPC := 12
    // tagLength := 24 { Panjang tag yang utuh}
  // Variabel
    // mode : integer {0 : baca data, 1 : tampilkan data}
    // tagValid : integer { menyatakan tag yang terbaca dan sudah dihitung CRC-nya dan siap untuk dikirim }
    // tag : integer { menyatakan tag yang terbaca }
    // flag_COMPLETE_ONE_CYCLE { menyatakan selesai satu siklus baca-tampil}
    // flag_COMPLETE_DISPLAY { menyatakan selesai tampil }
    // flag_COMPLETE_READ { menyatakan selesai baca }
  // Command
    // cmd_readDevice : baca alamat devais
    // cmd_setFreqRegion : atur region frekuensi
    // cmd_setFreqCh : atur channel frekuensi
    // cmd_autoFreqHop : automatic frequency hopping (ON)
    // cmd_PowEmmCap : atur kapasitas emisi daya 30 dBm
    // cmd_setWorkParam : jumlah antenna 1, port 1, polling open, power 30 dBm, batas kali bacaan per antenna 200
    // cmd_startRead : mulai baca
    // cmd_stopRead : berhenti baca
    /* cara kirim perintah ke RFID :
          >>> Serial2.write(cmdx, sizeof(cmdx));   x : nomor perintah
    */
  // Fungsi/Prosedur
    // compareTags(tag1, tag2, start, end) -> bool
      // memeriksa duplikasi tag
    // Calculate_CRC(ptr, len) -> integer (unsigned)
      // menghitung CRC
    // procedure reinitVariabeles
      // inisiasi ulang variabel setelah satu siklus

// PINOUT
  //    PSOC6 CY8CPROTO-062-4343W     <->     x
  // ================================
  //    D25       <->   LED
  //    D13       <->   button
  //    D16 (Rx)  <->   RFID Scanner (Tx)
  //    D17 (Tx)  <->   RFID Scanner (Rx)

// Programmer :
	// Bostang Palaguna
// Development Date
	// 2023/12/10

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Pinout
*******************************************************************************/
#define rx_rfid_pin CYBSP_DEBUG_UART_TX
#define tx_rfid_pin CYBSP_DEBUG_UART_RX
#define pb_pin CYBSP_USER_BTN
#define led_pin CYBSP_USER_LED

/*******************************************************************************
* Konstanta
*******************************************************************************/
#define READ_TAG_MODE 0
#define DISPLAY_TAG_MODE 1
#define tagLength 24
#define DEBOUNCE_DURATION 300
const int maxTag = 100;
const int maxRespByte = 30;
const int lenEPC = 12;
const byte header[] = { 0xAA, 0xAA, 0xFF };

/*******************************************************************************
* Macros
*******************************************************************************/
#define DELAY_SHORT_MS          (250)   /* milliseconds */
#define DELAY_LONG_MS           (500)   /* milliseconds */
#define LED_BLINK_COUNT         (4)
#define GPIO_INTERRUPT_PRIORITY (7u)

/*******************************************************************************
* Commands
*******************************************************************************/
//command untuk membaca device address
const byte cmd_readDevice[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x05, 0x01, 0xFF, 0x1B, 0xA2 };
//command untuk setting frequency region 920.125~924.875MHz
const byte cmd_setFreqRegion[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x30, 0x00, 0x01, 0x08, 0x17 };
//command untuk setting work frequency channel
const byte cmd_setFreqCh[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x32, 0x00, 0x00, 0x76, 0x56 };
//command untuk turn Automatic Frequency Hopping Mode on
const byte cmd_autoFreqHop[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x37, 0x00, 0xFF, 0x83, 0x56 };
//command untuk Set RF Emission Power Capacity 0BB8 = 3000 = 30 dBm
//byte cmd_PowEmmCap[] = {0xAA,0xAA,0xFF,0x07,0x3B,0x00,0x0B,0xB8,0xEB,0x5E};
//command untuk Set RF Emission Power Capacity 14 dBm
//byte cmd_PowEmmCap[] = { 0xAA, 0xAA, 0xFF, 0x07, 0x3B, 0x00, 0x05, 0x78, 0x11, 0x1D };
//command untuk Set RF Emission Power Capacity 20 dBm
//byte cmd_PowEmmCap[] = {0xAA,0xAA,0xFF,0x07,0x3B,0x00,0x07,0xD0,0x43,0x9D};
//command untuk Set RF Emission Power Capacity 30 dBm
const byte cmd_PowEmmCap[] = { 0xAA, 0xAA, 0xFF, 0x07, 0x3B, 0x00, 0x0B, 0xB8, 0xEB, 0x5E };
// command untuk single tag inventory
// byte cmd6[] = {0xAA,0xAA,0xFF,0x05,0xC8,0x00,0x3A,0x5E};
// command untuk set working parameters of the antenna
// jumlah antenna 1, port 1, polling open, power 30 dBm, batas kali bacaan per antenna 10
//byte cmd_setWorkParam[] = {0xAA, 0xAA, 0xFF, 0x0E, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB8, 0x0A, 0x21, 0xB2};
// jumlah antenna 1, port 1, polling open, power 10 dBm, batas kali bacaan per antenna 10
//byte cmd_setWorkParam[] = {0xAA, 0xAA, 0xFF, 0x0E, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0xE8, 0x0A, 0x86, 0xAC};
// jumlah antenna 1, port 1, polling open, power 14 dBm, batas kali bacaan per antenna 10
//byte cmd_setWorkParam[] = {0xAA, 0xAA, 0xFF, 0x0F, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x05, 0x78, 0x00, 0x0A, 0x09, 0x0C};
// jumlah antenna 1, port 1, polling open, power 20 dBm, batas kali bacaan per antenna 200
//byte cmd_setWorkParam[] = {0xAA, 0xAA, 0xFF, 0x0F, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x07, 0xD0, 0x00, 0xC8, 0x09, 0x57};
// jumlah antenna 1, port 1, polling open, power 30 dBm, batas kali bacaan per antenna 200
const byte cmd_setWorkParam[] = { 0xAA, 0xAA, 0xFF, 0x0F, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB8, 0x00, 0xC8, 0x74, 0xAF };
// command untuk start multiple tags in inventory
// mulai multiple read
const byte `cmd_startRead[] = { 0xAA, 0xAA, 0xFF, 0x08, 0xC1, 0x00, 0x08, 0x00, 0x00, 0x60, 0x4A };
//menghentikan multiple read
const byte cmd_stopRead[] = { 0xAA, 0xAA, 0xFF, 0x05, 0xC0, 0x00, 0xB3, 0xF7 };
//reset reader
// byte cmd_readDevice0[] = { 0xAA, 0xAA, 0xFF, 0x05, 0x0F, 0x00, 0xB5, 0x9D };

/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool gpio_intr_flag = false;
cyhal_gpio_callback_data_t gpio_btn_callback_data;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);


/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: mainTask
********************************************************************************/
void blinky(void * arg)
{
    (void)arg;

    /* Initialize the LED GPIO pin */
    cyhal_gpio_init(LED_GPIO, CYHAL_GPIO_DIR_OUTPUT,
                    CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    for(;;)
    {
        /* Toggle the LED periodically */
        cyhal_gpio_toggle(LED_GPIO);
        vTaskDelay(500);
    }
}

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
    uint32_t count = 0;
    uint32_t delay_led_blink = DELAY_LONG_MS;

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

    /* Initialize the user LED */
       result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
                       CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
       /* User LED init failed. Stop program execution */
       handle_error(result);

       /* Initialize the user button */
       result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
                       CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
       /* User button init failed. Stop program execution */
       handle_error(result);

       /* Configure GPIO interrupt */
       gpio_btn_callback_data.callback = gpio_interrupt_handler;
       cyhal_gpio_register_callback(CYBSP_USER_BTN,
                                    &gpio_btn_callback_data);
       cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL,
                                    GPIO_INTERRUPT_PRIORITY, true);

       /* Enable global interrupts */
       __enable_irq();


    // Proses pembacaan data dari UART

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("***********************************************************\r\n");
    printf("HAL: RFID UART Transmit and Receive\r\n");
    printf("***********************************************************\r\n\n");

    __enable_irq();
 
    for (;;)
    {

    	 /* Check the interrupt status */
		if (true == gpio_intr_flag)
		{
			gpio_intr_flag = false;

			/* Proses yang dilakukan saat button ditekan */
			if (DELAY_LONG_MS == delay_led_blink)
			{
				delay_led_blink = DELAY_SHORT_MS;
			}
			else
			{
				delay_led_blink = DELAY_LONG_MS;
			}
		}

        if (CY_RSLT_SUCCESS == cyhal_uart_getc(&cy_retarget_io_uart_obj,
                                               &read_data, 0))
        {
            if (CY_RSLT_SUCCESS != cyhal_uart_putc(&cy_retarget_io_uart_obj,
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
