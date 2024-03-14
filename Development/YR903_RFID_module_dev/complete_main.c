/*******************************************************************************
* File Name:   main.c
*
* Description: Terdiri dari tiga buah state : IDLE, READ, DISPLAY.
* 	state idle -> tidak melakukan apa-apa
* 	state read -> mengirimkan perintah baca ke rfid scanner dan menyimpan ke sebuah array epc
* 	state display -> menampilkan data hasil bacaan rfid ke terminal, mengosongkan kembali array epc
*
*
********************************************************************************

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define DELAY_SHORT_MS          (250)   /* milliseconds */
#define DELAY_LONG_MS           (500)   /* milliseconds */
#define LED_BLINK_COUNT         (4)
#define GPIO_INTERRUPT_PRIORITY (7u)

#define STATE_IDLE 0
#define STATE_READ 1
#define STATE_DISPLAY 2

#define DEBOUNCE_DURATION 200u

#define DATA_BITS_8     8
#define STOP_BITS_1     1
#define BAUD_RATE       115200
#define UART_DELAY      10u
#define RX_BUF_SIZE     1
#define TX_BUF_SIZE     6
#define LEN_EPC			12 // 12 double


/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool gpio_intr_flag = false;
cyhal_gpio_callback_data_t gpio_btn_callback_data;

// FSM State
int state = STATE_IDLE; // initial state

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

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);


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
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function configures and initializes the GPIO
*  interrupt, update the delay on every GPIO interrupt, blinks the LED and enter
*  in deepsleep mode.
*
* Return: int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    /* Board init failed. Stop program execution */
    handle_error(result);

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);
    /* Retarget-io init failed. Stop program execution */
    handle_error(result);

    /* Initialize the user LED */
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
                    CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    /* User LED init failed. Stop program execution */
    handle_error(result);

    /* Initialize the user button */
    result = cyhal_gpio_init(P13_4, CYHAL_GPIO_DIR_INPUT,
    		CYHAL_GPIO_DRIVE_PULL_NONE , 0);

    /* User button init failed. Stop program execution */
    handle_error(result);

    /* Configure GPIO interrupt */
    gpio_btn_callback_data.callback = gpio_interrupt_handler;
    cyhal_gpio_register_callback(P13_4,
                                 &gpio_btn_callback_data);
    cyhal_gpio_enable_event(P13_4, CYHAL_GPIO_IRQ_RISE,
                                     GPIO_INTERRUPT_PRIORITY, true);

    /* Enable global interrupts */
    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("**************** HAL: GPIO Interrupt ****************\r\n");

    for (;;)
    {
        /* Check the interrupt status */
    	// Saat button ditekan
        if (true == gpio_intr_flag)
        {
            gpio_intr_flag = false;

            if (state == STATE_IDLE)
            {
            	state = STATE_READ; // next state
            	printf("State Read\r\n");
				cyhal_gpio_write(CYBSP_USER_LED, 1);
				cyhal_system_delay_ms(DEBOUNCE_DURATION);
            }
            else if (state == STATE_READ)
            {
            	state = STATE_DISPLAY;
            	printf("State Display\r\n");
            	cyhal_gpio_write(CYBSP_USER_LED, 0);
            	cyhal_system_delay_ms(DEBOUNCE_DURATION);
            }
            else
            {
            	state = STATE_IDLE;
            	printf("State Idle\r\n");
            	cyhal_gpio_write(CYBSP_USER_LED, 0);
            	cyhal_system_delay_ms(DEBOUNCE_DURATION);
            }
//            else // state_debounce_1
//            {
//            	cyhal_system_delay_ms(DEBOUNCE_DURATION);
//            	state = STATE0; // next state
//            }

//            /* Update LED toggle delay */
//            if (DELAY_LONG_MS == delay_led_blink)
//            {
//                delay_led_blink = DELAY_SHORT_MS;
//            }
//            else
//            {
//                delay_led_blink = DELAY_LONG_MS;
//            }
//        }
//
//        /* Blink LED four times */
//        for (count = 0; count < LED_BLINK_COUNT; count++)
//        {
//            cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
//            cyhal_system_delay_ms(delay_led_blink);
//            cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
//            cyhal_system_delay_ms(delay_led_blink);
//        }

        /* Enter deep sleep mode */
        cyhal_syspm_deepsleep();
        }
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


/* [] END OF FILE */
