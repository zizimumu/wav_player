#ifndef __WM_H
#define __WM_H

#include "stm32f4xx_conf.h"


void wm_8731_init(u32 sample,u32 frame_bits );
void wm_8731_reset(void);
void wm_8731_vol_down(void);

void wm_8731_vol_up(void);



#endif
