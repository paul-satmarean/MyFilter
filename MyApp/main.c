//
//   Author(s)    : Radu PORTASE(rportase@bitdefender.com)
//
#include "driver_commands.h"
#include "globaldata.h"
#include <stdio.h>

APP_GLOBAL_DATA gApp;

int 
__cdecl
main(
    int argc,
    char *argv[]
)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    CommDriverPreinitialize();
    NTSTATUS status = CommDriverInitialize();
    if (status < 0)
    {
        return status;
    }
	
    status = CmdStartMonitoring(notificationFile); //notificationProcess | notificationThread);
	printf("Start monitoring returned status = 0x%X\n", status);

	printf("Waiting for key...\n");
	char c;
	scanf_s("%c", &c, 1);
    
    status = CmdStopMonitoring(notificationFile);//notificationProcess | notificationThread);
	printf("Stop monitoring returned status = 0x%X\n", status);

    CommDriverUninitialize();
}