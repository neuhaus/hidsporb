//
// io.h
//

NTSTATUS
OrbStartIo(IN PDEVICE_EXTENSION devExt);

VOID
OrbStopIo(IN PDEVICE_EXTENSION devExt);

VOID
OrbReadThread(IN PDEVICE_OBJECT devObj);
