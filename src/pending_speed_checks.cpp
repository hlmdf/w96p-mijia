#include "pending_speed.h"
constexpr auto on=mergeSpeed(PendingSpeed(),-1,1,100,1);
static_assert(on.target==1 && !on.ready(749) && on.ready(750),"Bare on restores remembered speed only after grace period");
constexpr auto percent=mergeSpeed(on,43,1,150,2);
static_assert(percent.target==43 && percent.ready(230),"1/off/on/43 must replace the remembered value with 43");
constexpr auto lateOn=mergeSpeed(percent,-1,1,170,3);
static_assert(lateOn.target==43 && lateOn.due==230 && lateOn.revision==3,"Late bare on cannot overwrite an explicit percentage");
constexpr auto ten=mergeSpeed(mergeSpeed(PendingSpeed(),-1,100,100,1),10,100,110,2);
static_assert(ten.target==10,"Old 100 percent cannot precede requested 10 percent");
constexpr auto latest=mergeSpeed(percent,21,1,180,4);
static_assert(latest.target==21,"Rapid percent updates preserve the newest target");
constexpr auto wrap=mergeSpeed(PendingSpeed(),10,1,0xfffffff0U,1);
static_assert(!wrap.ready(0x30U) && wrap.ready(0x40U),"Deadlines survive millis wraparound");
static_assert(companionOn(-1,43,300,150),"A delayed companion on cannot restore old speed after percent dispatch");
static_assert(!companionOn(-1,-1,300,150),"Off clears percent companion suppression");
static_assert(!companionOn(-1,43,900,150),"A later independent on remains available");
