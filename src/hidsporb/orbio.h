//
// orbio.h
//

NTSTATUS
OrbInitComPort(IN PDEVICE_OBJECT devObj);

NTSTATUS
OrbPowerUp(IN PDEVICE_OBJECT devObj);

NTSTATUS
OrbPowerDown(IN PDEVICE_OBJECT devObj);
