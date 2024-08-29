#ifndef __SYN6288_H
#define __SYN6288_H

#include "ftypes.h"

void syn6288UartInit(void);
// void syn6288Play(void);
void syn6288Play(u8 *data);
void SYN_FrameInfo(u8 Music, u8 *HZdata);
void YS_SYN_Set(u8 *Info_data);

#endif

