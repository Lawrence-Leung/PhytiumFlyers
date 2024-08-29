#include "stdio.h"
#include "shell_port.h"
#include "lwip/apps/lwiperf.h"
#include "sdkconfig.h"


static int lwiperf_test(int argc, char *argv[])
{
    static int start_flg = 0 ;

    if(start_flg != 0)
    { 
        printf("lwiperf_test is already running \r\n") ; 
    }
 
    start_flg++;
    lwiperf_start_tcp_server_default(NULL,NULL);
    return 0;
} 
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), lwiperf_test, lwiperf_test, Setup LWIP device test);