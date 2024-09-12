#ifndef __MACRO_H
#define __MACRO_H 1

#define ATTR_PACK __attribute__((__packed__))
#define ATTR_REQUEST __attribute__((used, section(".requests")))

#include "color.h"

#define KDEBUG BLU "[DEBUG] " RESET
#define KWARN YEL "[WARN] " RESET
#define KPANIC RED "[PANIC] " RESET
#define KERROR RED "[ERROR] " RESET
#define KINFO CYN "[INFO] " RESET
#define KOK GRN "[ OK ] " RESET

#endif
