/*******************************************************************************
* File Name:   button.h (WMS v4.0)
* Description: implementasi fungsi/prosedur yang berkaitan dengan button
* Programmer: Bostang
* Tanggal : 2024/04/04
*
* Catatan v4.0 : - dibuat untuk PCB versi 4.0 yang menggunakan mekanisme hardware debouncing -> tidak perlu menggunakan delay untuk debounce.
* 				 - button bukan lagi active low tetapi active high (apabila ditekan, dia state 0V. tidak ditekan, 5V).
*******************************************************************************/
// Header Guard
#ifndef BUTTON_H
#define BUTTON_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
//#include "cyhal_clock.h"		// untuk short/long-pressed

#include "fsm.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define DEBOUNCING_DELAY_MS          (1000)   /* milliseconds */
#define GPIO_INTERRUPT_PRIORITY      (7u)
#define BTN_TASK_STACK_SIZE          (1024)
#define BTN_TASK_PRIORITY            (5)

#define BTN_ACQ P13_4
#define BTN_PREV P13_6
#define BTN_MODE P9_6

/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool gpio_intr_flag_acq = false;
volatile bool gpio_intr_flag_prev = false;
volatile bool gpio_intr_flag_mode = false;

cyhal_gpio_callback_data_t gpio_btn_acq_callback_data;
cyhal_gpio_callback_data_t gpio_btn_prev_callback_data;
cyhal_gpio_callback_data_t gpio_btn_mode_callback_data;

cyhal_clock_t system_clock;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void gpio_interrupt_handler_acq(void *handler_arg, cyhal_gpio_event_t event);
static void gpio_interrupt_handler_prev(void *handler_arg, cyhal_gpio_event_t event);
static void gpio_interrupt_handler_mode(void *handler_arg, cyhal_gpio_event_t event);

void button_task_acq(void *pvParameters);
void button_task_prev(void *pvParameters);
void button_task_mode(void* arg);

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: gpio_interrupt_handler_acq
********************************************************************************
* Summary:
*   GPIO interrupt handler for button acquire.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void gpio_interrupt_handler_acq(void *handler_arg, cyhal_gpio_event_t event)
{
    gpio_intr_flag_acq = true;
}

/*******************************************************************************
* Function Name: gpio_interrupt_handler_prev
********************************************************************************
* Summary:
*   GPIO interrupt handler for button previous.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void gpio_interrupt_handler_prev(void *handler_arg, cyhal_gpio_event_t event)
{
    gpio_intr_flag_prev = true;
}

/*******************************************************************************
* Function Name: gpio_interrupt_handler_mode
********************************************************************************
* Summary:
*   GPIO interrupt handler for button mode.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void gpio_interrupt_handler_mode(void *handler_arg, cyhal_gpio_event_t event)
{
    gpio_intr_flag_mode = true;
}

/*******************************************************************************
* Function Name: button_task_acq
********************************************************************************
* Summary:
*   Task to handle events for button acquire.
*
* Parameters:
*   void *arg: Task arguments (unused)
*
*******************************************************************************/
void button_task_acq(void *pvParameters)
{
    TaskParameters *params = (TaskParameters *)pvParameters;
    int* state = &(params->state);

    printf("Button Task Acquire started!\r\n");

    for (;;)
    {
        /* Check the interrupt status */
        if (gpio_intr_flag_acq)
        {
            gpio_intr_flag_acq = false;
            printf("Button Acquire pressed!\r\n");
            // Lock the mutex before accessing the shared state
			if (xSemaphoreTake(xStateMutex, portMAX_DELAY) == pdTRUE)
			{
				// perpindahan state
				*state = handle_acquire_button(state);

	            // menampilkan state saat ini
	            display_state(state);

				// Release the mutex after accessing the shared state
                xSemaphoreGive(xStateMutex);
			}

        }
//        vTaskDelay(pdMS_TO_TICKS(DEBOUNCING_DELAY_MS)); // Debouncing delay
        taskYIELD(); // pindah ke task lain (cooperative scheduling)
    }
}

/*******************************************************************************
* Function Name: button_task_prev
********************************************************************************
* Summary:
*   Task to handle events for button previous.
*
* Parameters:
*   void *arg: Task arguments (unused)
*
*******************************************************************************/
void button_task_prev(void *pvParameters)
{
	TaskParameters *params = (TaskParameters *)pvParameters;
	int* state = &(params->state);

    printf("Button Task Previous started!\r\n");

    for (;;)
    {
        /* Check the interrupt status */
        if (gpio_intr_flag_prev)
        {
            gpio_intr_flag_prev = false;
//            cyhal_syspm_sleep();
            printf("Button Previous pressed!\r\n");

//             Lock the mutex before accessing the shared state
            if (xSemaphoreTake(xStateMutex, portMAX_DELAY) == pdTRUE)
            {
				// perpindahan state
				*state = handle_previous_button(state);

				printf("setelah handle_previous_button!\r\n");

				// menampilkan state saat ini
				display_state(state);

                // Release the mutex after accessing the shared state
                xSemaphoreGive(xStateMutex);
            }

        }

//        vTaskDelay(pdMS_TO_TICKS(DEBOUNCING_DELAY_MS)); // Debouncing delay
        taskYIELD(); // pindah ke task lain (cooperative scheduling)
    }
}

/*******************************************************************************
* Function Name: button_task_mode
********************************************************************************
* Summary:
*   Task to handle events for button mode.
*	12 Juni : untuk mengubah
* Parameters:
*   void *arg: Task arguments (unused)
*
*******************************************************************************/
void button_task_mode(void* arg)
{
    (void)arg;
    printf("Button Task Mode started!\r\n");

    for (;;) {
        /* Check the interrupt status */
        if (gpio_intr_flag_mode) {
            gpio_intr_flag_mode = false;  // diubah setelah tampil pada OLED

            // Cetak pesan ketika tombol ditekan
            printf("Button Mode pressed!\r\n");

            // Cetak nilai AWAL dari variabel
//			printf("ANYTHINGRESOURCE awal: %s\r\n", ANYTHINGRESOURCE); // DEBUG

            // Panggil fungsi untuk mengubah nilai variabel
            toggle_resource_path();

            // Cetak nilai baru dari variabel
            printf("ANYTHINGRESOURCE sekarang: %s\r\n", ANYTHINGRESOURCE); // DEBUG

            xTaskNotifyGive(oled_taskHandle);

            // Tambahkan delay untuk memberikan waktu bagi OLED task
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        taskYIELD(); // pindah ke task lain (cooperative scheduling)
    }
}


#endif /* BUTTON_H */

/* [] END OF FILE */
