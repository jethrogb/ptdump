/*
 * Page table dumper.
 *
 * (C) Copyright 2016 Jethro G. Beekman
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#ifndef PTDUMP_COMMON_H
#define PTDUMP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define pte_address(p) ((p)&0x000ffffffffff000UL)

typedef unsigned long long ullong;

// you need to define these
void output(void* p, void* data, ullong len);
ullong get_base(void* p);
ullong* read_page(void* p, ullong address);
void free_page(void* p, ullong* page);

// call this
void ptdump(void *p, int levels);

#ifdef __cplusplus
}
#endif

#endif //PTDUMP_COMMON_H
