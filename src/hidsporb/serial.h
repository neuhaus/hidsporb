//
// serial.h
//

NTSTATUS
OrbSerSyncIoctl(IN PDEVICE_OBJECT devObj,
		IN BOOLEAN Internal,
		IN ULONG Ioctl,
		IN PKEVENT event,
		IN PIO_STATUS_BLOCK Iosb);

NTSTATUS
OrbSerSyncIoctlEx(IN PDEVICE_OBJECT devObj,
		IN BOOLEAN Internal,
		IN ULONG Ioctl,
		IN PKEVENT event,
		IN PIO_STATUS_BLOCK Iosb,
		IN PVOID inBuffer,
		IN ULONG inBufferLen,
		IN PVOID outBuffer,
		IN ULONG outBufferLen);

NTSTATUS
OrbSerRead(IN PDEVICE_OBJECT devObj, IN PIRP readIrp, OUT PCHAR readBuffer, IN ULONG bufSize, OUT PULONG bufRead);

NTSTATUS
OrbSerReadChar(IN PDEVICE_OBJECT devObj, IN PIRP readIrp, OUT PCHAR pChar);

NTSTATUS
OrbSerWrite(IN PDEVICE_OBJECT devObj, IN PIRP writeIrp, IN PCHAR writeBuffer, IN ULONG bufSize, OUT PULONG bufWritten);

NTSTATUS
OrbSerWriteChar(IN PDEVICE_OBJECT devObj, IN PIRP writeIrp, IN CHAR Char);

//
NTSTATUS
OrbSerOpenPort(IN PDEVICE_OBJECT devObj, IN PIRP openIrp);

NTSTATUS
OrbSerClosePort(IN PDEVICE_OBJECT devObj, IN PIRP closeIrp);

//
NTSTATUS
OrbSerGetLineControl(IN PDEVICE_OBJECT devObj, PSERIAL_LINE_CONTROL lineControl);

NTSTATUS
OrbSerSetLineControl(IN PDEVICE_OBJECT devObj, PSERIAL_LINE_CONTROL lineControl);

NTSTATUS
OrbSerGetTimeouts(IN PDEVICE_OBJECT devObj, PSERIAL_TIMEOUTS timeouts);

NTSTATUS
OrbSerSetTimeouts(IN PDEVICE_OBJECT devObj, PSERIAL_TIMEOUTS timeouts);

NTSTATUS
OrbSerGetBaudRate(IN PDEVICE_OBJECT devObj, PSERIAL_BAUD_RATE baudRate);

NTSTATUS
OrbSerSetBaudRate(IN PDEVICE_OBJECT devObj, PSERIAL_BAUD_RATE baudRate);

NTSTATUS
OrbSerGetHandflow(IN PDEVICE_OBJECT devObj, PSERIAL_HANDFLOW handFlow);

NTSTATUS
OrbSerSetHandFlow(IN PDEVICE_OBJECT devObj, PSERIAL_HANDFLOW handFlow);

NTSTATUS
OrbSerGetDtr(IN PDEVICE_OBJECT devObj, PULONG Dtr);

NTSTATUS
OrbSerGetRts(IN PDEVICE_OBJECT devObj, PULONG Rts);

NTSTATUS
OrbSerSetDtr(IN PDEVICE_OBJECT devObj);

NTSTATUS
OrbSerClrDtr(IN PDEVICE_OBJECT devObj);

NTSTATUS
OrbSerSetRts(IN PDEVICE_OBJECT devObj);

NTSTATUS
OrbSerClrRts(IN PDEVICE_OBJECT devObj);

NTSTATUS
OrbSerFlush(IN PDEVICE_OBJECT devObj, IN ULONG Mask);
