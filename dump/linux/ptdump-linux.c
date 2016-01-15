/*
 * Page table dumper in Linux procfs.
 *
 * (C) Copyright 2016 Jethro G. Beekman
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/io.h>

#ifdef CONFIG_X86_64

#undef free_page
#include "ptdump-common.h"

void output(void* p, void* data, ullong len) {
	seq_write((struct seq_file *)p, data, len);
}

ullong get_base(void* p) {
	return read_cr3();
}

ullong* read_page(void* p, ullong address) {
	return (ullong*)phys_to_virt(address);
}

void free_page(void* p, ullong* page) {
}

static int do_ptdump(struct seq_file *m, void* v) {
	ptdump(m,(int)(unsigned long)(m->private));
	return 0;
}

static int ptdump_open(struct inode *inode, struct file *fp) {
	return single_open(fp, do_ptdump, PDE_DATA(inode));
}

static const struct file_operations ptdump_fops = {
	.open    = ptdump_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};

static struct proc_dir_entry *proc_page_table0;
static struct proc_dir_entry *proc_page_table1;
static struct proc_dir_entry *proc_page_table2;
static struct proc_dir_entry *proc_page_table3;

#endif

static int __init ptdump_module_init(void) {
#ifdef CONFIG_X86_64
	proc_page_table0=proc_create_data("page_table_0", 0444, NULL, &ptdump_fops,(void*)0);
	proc_page_table1=proc_create_data("page_table_1", 0444, NULL, &ptdump_fops,(void*)1);
	proc_page_table2=proc_create_data("page_table_2", 0444, NULL, &ptdump_fops,(void*)2);
	proc_page_table3=proc_create_data("page_table_3", 0444, NULL, &ptdump_fops,(void*)3);
#endif
	return 0;
}

static void __exit ptdump_module_exit(void) {
#ifdef CONFIG_X86_64
	proc_remove(proc_page_table0);
	proc_remove(proc_page_table1);
	proc_remove(proc_page_table2);
	proc_remove(proc_page_table3);
#endif
}

module_init(ptdump_module_init);
module_exit(ptdump_module_exit);
MODULE_LICENSE("GPL");
