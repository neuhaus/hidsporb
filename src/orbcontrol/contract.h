#ifndef CONTRACT_H
#define CONTRACT_H

#include <assert.h>

//@doc

#ifndef NO_PRECONDITIONS
#ifndef REQUIRE
#define REQUIRE( flag ) { assert( flag ); };
#endif
#else
#ifndef REQUIRE
#define REQUIRE( flag )
#endif
#endif

#ifndef NO_CHECKS
#ifndef CHECK
#define CHECK( flag ) { assert( flag ); };
#endif
#else
#ifndef CHECK
#define CHECK( flag )
#endif
#endif

#ifndef NO_POSTCONDITIONS
#ifndef ENSURE
#define ENSURE( flag ) { assert( flag ); };
#endif
#else
#ifndef ENSURE
#define ENSURE( flag )
#endif
#endif

#endif

