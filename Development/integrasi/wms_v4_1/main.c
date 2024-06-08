/*******************************************************************************
* File Name:   main.c (WMS v4.1)
* Description: Melakukan pengujian integrasi antara sub-sistem : Button ditekan, maka akan menjalankan barcode scanner
* 				disertai dengan bunyi buzzer dan LED
* Programmer: Bostang
* Tanggal : 2024/05/08
* Catatan Perubahan v2.0:
* 	- dari pre-emptive scheduling diubah menjadi cooperative scheduling supaya suatu task bisa sampai selesai melakukan pekerjaannya baru berpindah ke task lain
* 	- barcode scanner tidak menyala secara default, tetapi dikirim perintah untuk mulai baca (menghemat energi serta mencegah IC barcode scanner panas)
* 	- terintegrasi dari scanning sampai kirim data ke back-end
*
* 	Improvement untuk v2.1 :
* 		TO-DO LIST :
* 	- multiple cycle scanning (DONE)
* 	- scanning lalu simpan ke array sehingga nanti jumlah data yang bisa dikirim lebih dari satu tag (DONE)
* 	- fix Bug : buzzer kadang-kadang berdering sekali atau terus berdering (ekspektasi hanya dua kali). Ini kemungkinan masalah scheduling
* 	- setelah data sukses dikirim, maka di OLED menampilkan pesan sukses.
* 		DONE :
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cyabs_rtos.h"

// include file libary yang di-deklarasi-kan sendiri
#include "button.h"
#include "utils.h"
#include "led.h"
#include "oled.h"
#include "buzzer.h"

#include "mtb_ssd1306.h"
#include "GUI.h"

#include "http_client.h"

#include "data_acquisition.h"

#include "fsm.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/
cy_rslt_t result;

int state;
TaskParameters taskParams =
{
   .state = STATE_IDLE
//   .epc= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

SemaphoreHandle_t Event_Semaphore;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void initialize_buttons_and_interrupts();
void tasks_creation();

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  Entry function for the application.
*  This function configures and initializes the GPIO
*  interrupt.
*
* Return: int
*
*******************************************************************************/
int main(void)
{
	// membuat semaphore supaya task http berjalan setelah selesai terima data dan tampil ke OLED
	Event_Semaphore = xSemaphoreCreateBinary();
	if (Event_Semaphore == NULL)
	{
		printf("Semaphore cannot be created");
	}

    // Create the mutex before starting tasks
    xStateMutex = xSemaphoreCreateMutex();
	if (xStateMutex == NULL)
	{
		printf("Semaphore cannot be created");
	}

    /* INISIASI KOMPONEN, INTERRUPT, DAN TASK */
    initialize_buttons_and_interrupts();

    /* Enable global interrupts */
    __enable_irq();

    // Buat queue
    // queue dari data acquisition task ke display task
    dataQueue = xQueueCreate(1, sizeof(uint8_t[TX_BUF_SIZE]));
    if (dataQueue == NULL)
    {
        printf("Failed to create queue.\r\n");
        while(1);
    }

    // queue dari display task ke http client task (array)

    dataQueue_array = xQueueCreate(MAX_EPC_ARRAY_SIZE, sizeof(tags));
    if (dataQueue_array == NULL)
    {
        printf("Failed to create queue.\r\n");
        while(1);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("**************** WMSv4.1 testing ****************\r\n");

    /* MEMBUAT TASK DAN MEMULAI SCHEDULER */
    tasks_creation();

    for (;;)
    {
        /* Code should not reach here */
    }
}

/*******************************************************************************
* Function Name: initialize_buttons_and_interrupts()
********************************************************************************
* Summary: Prosedur untuk inisiasi button dan interruptnya
*
* Return: -
*
*******************************************************************************/
void initialize_buttons_and_interrupts()
{
	/* Initialize the device and board peripherals */
	result = cybsp_init();
	/* Board init failed. Stop program execution */
	handle_error(result);

	/* Initialize retarget-io to use the debug UART port */
	result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
								 CY_RETARGET_IO_BAUDRATE);
	/* Retarget-io init failed. Stop program execution */
	handle_error(result);

	/* Initialize the user buttons */
	// button acquire
	result = cyhal_gpio_init(BTN_ACQ, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
	/* button init failed. Stop program execution */
	handle_error(result);

	// button previous
	result = cyhal_gpio_init(BTN_PREV, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
	handle_error(result);

	// button mode
	result = cyhal_gpio_init(BTN_MODE, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
	handle_error(result);

	/* Configure GPIO interrupts */
	gpio_btn_acq_callback_data.callback = gpio_interrupt_handler_acq;
	cyhal_gpio_register_callback(BTN_ACQ,&gpio_btn_acq_callback_data);
	cyhal_gpio_enable_event(BTN_ACQ, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true); //button active-low

	gpio_btn_prev_callback_data.callback = gpio_interrupt_handler_prev;
	cyhal_gpio_register_callback(BTN_PREV, &gpio_btn_prev_callback_data);
	cyhal_gpio_enable_event(BTN_PREV, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);  //button active-low

	gpio_btn_mode_callback_data.callback = gpio_interrupt_handler_mode;
	cyhal_gpio_register_callback(BTN_MODE,&gpio_btn_mode_callback_data);
	cyhal_gpio_enable_event(BTN_MODE, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true); //button active-low

	/* Inisiasi OLED : dilakukan di dalam task-nya */

	/* Inisiasi Buzzer */
    cyhal_gpio_init(BUZZ_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0U);

}

/*******************************************************************************
* Function Name: task_creation()
********************************************************************************
* Summary: Prosedur untuk membuat para task
*
* Return: -
*
*******************************************************************************/
void tasks_creation()
{
    int retval;

    /* Create tasks to handle button events */
	retval = xTaskCreate(button_task_acq, "Button Task Acquire", configMINIMAL_STACK_SIZE, (void *)&taskParams, BTN_TASK_PRIORITY-4, NULL);
	if (pdPASS != retval)
	{
		printf("Button Task Acquire creation failed!\r\n");
	}

	retval = xTaskCreate(button_task_prev, "Button Task Previous", configMINIMAL_STACK_SIZE, (void *)&taskParams, BTN_TASK_PRIORITY-4, NULL);
	if (pdPASS != retval)
	{
		printf("Button Task Previous creation failed!\r\n");
	}

	retval = xTaskCreate(button_task_mode, "Button Task Mode", configMINIMAL_STACK_SIZE, NULL, BTN_TASK_PRIORITY-4, NULL);
	if (pdPASS != retval)
	{
		printf("Button Task Mode creation failed!\r\n");
	}

	/* Create task to handle led events */
	retval = xTaskCreate( red_led_task, RED_LED_TASK_NAME,configMINIMAL_STACK_SIZE, (void *)&taskParams, LED_TASK_PRIORITY, NULL );

	if( pdPASS != retval )
	{
		 printf("%s did not start!\r\n", RED_LED_TASK_NAME);
	}

	retval = xTaskCreate( green_led_task, GREEN_LED_TASK_NAME,configMINIMAL_STACK_SIZE, (void *)&taskParams , LED_TASK_PRIORITY, NULL );
	if (pdPASS != retval)
	{
		printf("Green LED Task Mode creation failed!\r\n");
	}

	/* Create task to handle oled events */
	retval = xTaskCreate( oled_task, OLED_TASK_NAME,OLED_TASK_STACK_SIZE, (void *)&taskParams, OLED_TASK_PRIORITY, &oled_taskHandle );
	if (pdPASS != retval)
	{
		printf("OLED Task Mode creation failed!\r\n");
	}

	/* Create task to handle data acquisition events */
	retval = xTaskCreate(data_acquisition_task, "Data Acquisition Task", DATA_ACQ_TASK_STACK_SIZE, (void *)&taskParams, DATA_ACQ_TASK_PRIORITY, &data_acq_taskHandle); // bug perbesar stack
	if (pdPASS != retval)
	{
		printf("OLED Task Mode creation failed!\r\n");
	}

	/* Create task to handle buzzer events */
	retval = xTaskCreate( buzzer_task, BUZZ_TASK_NAME,configMINIMAL_STACK_SIZE, NULL, BUZZ_TASK_PRIORITY, NULL );
	if (pdPASS != retval)
	{
		printf("Buzzer Task creation failed!\r\n");
	}

	/* Create the client task. */
	retval = xTaskCreate(http_client_task, "Network task", HTTP_CLIENT_TASK_STACK_SIZE, (void *)&taskParams, HTTP_CLIENT_TASK_PRIORITY+1, &client_task_handle);
	if (pdPASS != retval)
	{
		printf("HTTP Client Task creation failed!\r\n");
	}

	printf("Scheduler start!\r\n");

	/* Start FreeRTOS scheduler */
	vTaskStartScheduler();
}

/* [] END OF FILE */
