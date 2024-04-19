/*******************************************************************************
* File Name:   button.h (Integrasi 1)
* Description: implementasi fungsi/prosedur yang berkaitan dengan button
* Programmer: Bostang
* Tanggal : 2024/04/04
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

//#include "fsm.h"

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

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void gpio_interrupt_handler_acq(void *handler_arg, cyhal_gpio_event_t event);
static void gpio_interrupt_handler_prev(void *handler_arg, cyhal_gpio_event_t event);
static void gpio_interrupt_handler_mode(void *handler_arg, cyhal_gpio_event_t event);

void button_task_acq(void* arg);
void button_task_prev(void* arg);
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
void button_task_acq(void* arg)
{
    printf("Button Task Acquire started!\r\n");

    for (;;)
    {
        /* Check the interrupt status */
        if (gpio_intr_flag_acq)
        {
            gpio_intr_flag_acq = false;
            printf("Button Acquire pressed!\r\n");

//            // perpindahan task dengan FSM
//            *arg = handle_acquire_button(arg);

//            // menampilkan state saat ini
//            display_state(arg);
        }

        vTaskDelay(pdMS_TO_TICKS(DEBOUNCING_DELAY_MS)); // Debouncing delay
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
void button_task_prev(void* arg)
{
    (void)arg;

    printf("Button Task Previous started!\r\n");

    for (;;)
    {
        /* Check the interrupt status */
        if (gpio_intr_flag_prev)
        {
            gpio_intr_flag_prev = false;
            printf("Button Previous pressed!\r\n");

//            // perpindahan task dengan FSM
//            *arg = handle_previous_button(arg);
//
//            // menampilkan state saat ini
//            display_state(arg);
        }

        vTaskDelay(pdMS_TO_TICKS(DEBOUNCING_DELAY_MS)); // Debouncing delay
    }
}

/*******************************************************************************
* Function Name: button_task_mode
********************************************************************************
* Summary:
*   Task to handle events for button mode.
*
* Parameters:
*   void *arg: Task arguments (unused)
*
*******************************************************************************/
void button_task_mode(void* arg)
{
    (void)arg;

    printf("Button Task Mode started!\r\n");

    for (;;)
    {
        /* Check the interrupt status */
        if (gpio_intr_flag_mode)
        {
            gpio_intr_flag_mode = false;
            printf("Button Mode pressed!\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(DEBOUNCING_DELAY_MS)); // Debouncing delay
    }
}
