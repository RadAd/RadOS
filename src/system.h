void system_reboot();
void system_shutdown_qemu();
#pragma aux system_shutdown_qemu = \
    "mov ax, 2000h" \
	"mov dx, 604h" \
	"out dx, ax";
