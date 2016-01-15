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

#include "ptdump-common.h"

static ullong* ptdump_dump(void *p, ullong address) {
	ullong* page=read_page(p, address);
	if (page) {
		output(p, &address, 8);
		output(p, page, 4096);
	}
	return page;
}

static void ptdump_page(void *p, ullong address, int levels_remaining);

static void ptdump_recurse(void *p, ullong* page, int levels_remaining) {
	int i;
	for (i=0;i<512;i++) {
		if ((page[i]&1) && !(page[i]&(1<<7)))
			ptdump_page(p,pte_address(page[i]),levels_remaining-1);
	}
}

static void ptdump_page(void *p, ullong address, int levels_remaining) {
	ullong* page=ptdump_dump(p,address);
	if (!page) return;
	if (levels_remaining>0)
		ptdump_recurse(p,page,levels_remaining);
	free_page(p,page);
}

void ptdump(void *p, int levels) {
	ptdump_page(p,get_base(p),levels);
}
