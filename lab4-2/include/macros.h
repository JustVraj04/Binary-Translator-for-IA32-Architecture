#pragma once

/*********************************************************************
 *
 * Useful macros
 *
 *********************************************************************/


#include <assert.h>
#include <stdint.h>

#define ASSERT              assert
#define NOT_REACHED()       assert("SHOULD NOT REACH"  && 0)
#define NOT_IMPLEMENTED()   assert("NOT IMPLEMENTED"   && 0)
#define DPRINT              printf

#define BIT(_v, _b)         (((_v) >> (_b)) & 0x1)
#define BITS(_v, _l, _h)    (((uint32_t)(_v) << (31-(_h))) >> ((_l)+31-(_h)))
