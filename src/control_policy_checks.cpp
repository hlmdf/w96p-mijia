#include "control_policy.h"
static_assert(motorOffWrite(0xfff1,1,0),"Local power-off must share coupled off behavior");
static_assert(motorOffWrite(0xfff3,1,0),"Zero percent must turn off the motor and light");
static_assert(!motorOffWrite(0xfffa,1,0),"Independent light-off cannot stop the fan");
static_assert(!motorOffWrite(0xfffa,1,4),"Independent light-on remains allowed while fan is off");
static_assert(motorBecameOff(1,43,0,0),"Physical/timed shutdown needs light-off coupling");
static_assert(!motorBecameOff(0,0,0,0),"Being off must not repeatedly override independent light control");
static_assert(!motorBecameOff(-1,-1,0,0),"Unknown startup state must not invent a shutdown action");
static_assert(!motorBecameOff(1,21,1,0),"A zero natural-wind point must not turn off the independent light");
