#ifndef  UART_H
#define  UART_H

#include <stdio.h>
#include "fpl011_os.h"


void UartWaitLoop(void);
void UartTaskCreate(void *args);

#endif
