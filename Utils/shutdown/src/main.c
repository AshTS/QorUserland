#include <libc/stdio.h>
#include <libc/sys/syscalls.h>

int main() 
{
	sys_reboot(REBOOT_MAGIC1, REBOOT_MAGIC2, REBOOT_CMD_HALT, NULL);
	eprintf("Shutdown failed.");
	return 0;
}
