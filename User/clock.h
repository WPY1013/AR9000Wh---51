#ifndef __CLOCK_H__
#define __CLOCK_H__

#define IRC_CLOCK_HZ       11059200UL
#define SYSTEM_CLOCK_DIV   1UL
#define SYSTEM_CLOCK_HZ    (IRC_CLOCK_HZ / SYSTEM_CLOCK_DIV)

void SystemClock_Init(void);

#endif
