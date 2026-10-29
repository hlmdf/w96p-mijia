#include "light_auto_off.h"
static_assert(!lightOffDue(true,14999,15000),"Keep light available before the deadline");
static_assert(lightOffDue(true,15000,15000),"Always extinguish after 15 seconds");
static_assert(!lightOffDue(false,60000,15000),"Explicit off cancels the timer");
static_assert(!lightOffDue(true,0xfffffff0U,0x100U),"Wrapped future deadline must not fire early");
static_assert(lightOffDue(true,0x100U,0x100U),"Wrapped deadline still expires");
