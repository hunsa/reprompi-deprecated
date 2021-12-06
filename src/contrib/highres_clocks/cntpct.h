/* Armv8 CNTPCT_EL0 high resolution clock for ReproMPI
 *
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#ifndef CNTPCT_H_INCLUDED
#define CNTPCT_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

#define HRT_GET_FREQUENCY(frq) asm volatile ("isb; mrs %0, cntfrq_el0" : "=r" (frq));
#define HRT_GET_TIMESTAMP(pct) asm volatile ("isb; mrs %0, cntpct_el0" : "=r" (pct));

#define HRT_INIT(print, freq) \
do {\
  if(print) printf("# initializing Armv8 physical timer\n"); \
  HRT_GET_FREQUENCY(freq); \
} while(0)

inline uint64_t cntpct(void)
{
    unit64_t pct;
    HRT_GET_TIMESTAMP(pct);
    return pct;
}

#endif // CNTPCT_H_INCLUDED
