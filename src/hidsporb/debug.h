/*

Copyright (c) 2001, Victor B. Putz
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.  

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

* Neither the name of the project nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef DEBUG_H
#define DEBUG_H

#undef  C_ASSERT
#define C_ASSERT(e) switch(0) case(e): case(0):

  /* file type debug masks */
#define FILE_HIDSPORB                0x00010000
#define FILE_PNP                    0x00020000
#define FILE_POLL                   0x00040000
#define FILE_IOCTL                  0x00080000
#define FILE_HID_REPORTS            0x00100000
#define FILE_TIMING                 0x00200000
#define FILE_SERIAL_ROUTINES        0x00400000
#define FILE_ORB_COMM               0x00800000
#define FILE_SETTINGS               0x01000000
#define FILE_CONTROL                0x02000000
#define FILE_IRP_QUEUE              0x04000000

  /* message type debug masks */
#define HSO_ERROR                 0x00000001
#define HSO_WARNING                  0x00000002
#define HSO_MESSAGE1                0x00000004
#define HSO_MESSAGE2               0x00000008
#define HSO_FUNCTION_ENTRY                0x00000010
#define HSO_FUNCTION_EXIT                 0x00000020


#define HSO_FUNCTION_EXIT_OK        0x00001000
#define HSO_DEFAULT_DEBUGLEVEL    0x0000001
#define HSO_POOL_TAG              ('maGH')

  /* Since DBG is defined in wdm.h, we should make sure it's defined 
     and nonzero */
#ifdef DBG
#if DBG
#define TRAP()  DbgBreakPoint()
#endif
#endif

#ifdef TRAP
extern ULONG debug_log_level;
#define HSO_LOG( _log_mask_,  _log_message_) \
  if( (((_log_mask_) & debug_log_level)) ) \
    { \
      DbgPrint("HSO:");\
      DbgPrint _log_message_  ; \
      DbgPrint("\n"); \
    }
#define HSO_LOG_IF( _log_mask_, _flag_, _log_message_ ) \
  if ( (((_log_mask_) & debug_log_level)) && (_flag_)) \
    { \
      DbgPrint( "HIDSPORB.SYS: "); \
      DbgPrint _log_message_; \
      DbgPrint( "\n" ); \
    }
#define HSO_EXITPROC(_log_mask_, _log_message_, nt_status) \
  if( ((_log_mask_)&HSO_FUNCTION_EXIT_OK) && !NT_SUCCESS(nt_status) ) \
    {\
      HSO_LOG( (_log_mask_|HSO_ERROR), \
               (_log_message_ "  nt_status(0x%x)", nt_status) ); \
    }\
  else \
    { \
      HSO_LOG((_log_mask_), (_log_message_ " nt_status(0x%x)", nt_status)); \
    }
#define ExAllocPool( Type, Size ) \
   ExAllocatePoolWithTag( Type, Size, HIDSPORB_POOL_TAG )
#else

#define HSO_LOG(_log_mask_,_log_message_)
#define HSO_LOG_IF( _log_mask_, _log_flag_, _log_message_ );
#define HSO_EXITPROC(_log_mask_,_log_message_,_nt_status_)
#define TRAP()
#define ExAllocPool( _type_, _size_ ) ExAllocatePool( _type_, _size_ )

#endif


#endif
