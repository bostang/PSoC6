#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define RX_BUF_SIZE     4
#define TX_BUF_SIZE     4

void data_acquisition_task(void *arg);
