#ifndef HIDSPORB_DATA_H
#define HIDSPORB_DATA_H

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

/*
  Number of axes and buttons
*/
#define NUM_AXES 6
#define NUM_BUTTONS 16

/*
min and max axis values.  These are defined as 0-1024 instead of -512 to +512
because of some guidance I saw saying that min values should be zero.  I
tried it both ways, and it worked fine both ways, but we'll stick to zero as
the minimum in any case.
*/
#define AXIS_MIN_VALUE 0 
#define AXIS_MAX_VALUE 1024 


/*
Structure describing orb input data: six axes and a button map.
*/

#include <pshpack1.h>

typedef struct _HIDSPORB_INPUT_DATA
{
  unsigned char report_id;
  ULONG   axes[NUM_AXES];
  USHORT  button_map;
} HIDSPORB_INPUT_DATA, *PHIDSPORB_INPUT_DATA;
typedef struct _HIDSPORB_INPUT_DATA UNALIGNED *PUHIDSPORB_INPUT_DATA;

#include <poppack.h>


#endif /* HIDSPORB_DATA_H */


