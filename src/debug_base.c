#include "debug_base.h"

#if DBG

static char* PnpMnString[] = {
  "IRP_MN_START_DEVICE",
  "IRP_MN_QUERY_REMOVE_DEVICE",
  "IRP_MN_REMOVE_DEVICE",
  "IRP_MN_CANCEL_REMOVE_DEVICE",
  "IRP_MN_STOP_DEVICE",
  "IRP_MN_QUERY_STOP_DEVICE",
  "IRP_MN_CANCEL_STOP_DEVICE",
  "IRP_MN_QUERY_DEVICE_RELATIONS",
  "IRP_MN_QUERY_INTERFACE",
  "IRP_MN_QUERY_CAPABILITIES",
  "IRP_MN_QUERY_RESOURCES",
  "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
  "IRP_MN_QUERY_DEVICE_TEXT",
  "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
  "IRP_MN_???",
  "IRP_MN_READ_CONFIG",
  "IRP_MN_WRITE_CONFIG",
  "IRP_MN_EJECT",
  "IRP_MN_SET_LOCK",
  "IRP_MN_QUERY_ID",
  "IRP_MN_QUERY_PNP_DEVICE_STATE",
  "IRP_MN_QUERY_BUS_INFORMATION",
  "IRP_MN_DEVICE_USAGE_NOTIFICATION",
  "IRP_MN_SURPRISE_REMOVAL",
  "IRP_MN_QUERY_LEGACY_BUS_INFORMATION"
};

#define	arraysize(a)	(sizeof((a)) / sizeof(a[0]))

char*
PnpToString(unsigned char minor)
{
  if (minor > arraysize(PnpMnString)) 
    {
      return "IRP_MN_???";
    }
  return PnpMnString[minor];
}

#endif
