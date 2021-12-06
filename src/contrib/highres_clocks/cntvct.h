/* Armv8 CNTVCT_EL0 high resolution clock for ReproMPI
 *
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#ifndef CNTVCT_H_INCLUDED
#define CNTVCT_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

#define HRT_GET_FREQUENCY(frq) asm volatile ("isb; mrs %0, cntfrq_el0" : "=r" (frq));
#define HRT_GET_TIMESTAMP(vct) asm volatile ("isb; mrs %0, cntvct_el0" : "=r" (vct));

#define HRT_INIT(print, freq) do {\
  if(print) printf("# initializing Armv8 virtual timer\n"); \
  HRT_GET_FREQUENCY(freq); \
} while(0)

inline uint64_t cntvct(void)
{
    unit64_t vct;
    HRT_GET_TIMESTAMP(vct);
    return vct;
}

#endif // CNTVCT_H_INCLUDED
