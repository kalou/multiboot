#ifndef _TIMER_H
#define _TIMER_H

u64 get_time(void);
void timer_irq(void);

void init_timer(void);

#endif
