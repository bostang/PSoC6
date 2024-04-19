/*******************************************************************************
* File Name:   utils.h (Integrasi 1)
* Description: implementasi fungsi-fungsi pembantu seperti error_handling
* Programmer: Bostang
* Tanggal : 2024/04/04
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdbool.h>
//#include "FreeRTOS.h"
//#include "task.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void handle_error(uint32_t status);

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
