//
// orbio.h
//

NTSTATUS
OrbInitComPort(IN PDEVICE_OBJECT devObj);

NTSTATUS
OrbPowerUp(IN PDEVICE_OBJECT devObj);

NTSTATUS
OrbPowerDown(IN PDEVICE_OBJECT devObj);

ULONG
OrbReadData(IN PDEVICE_OBJECT devObj, PCHAR buffer, IN ULONG size);
