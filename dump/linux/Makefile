obj-m += ptdump.o
ptdump-objs := ptdump-linux.o ../common/ptdump-common.o
EXTRA_CFLAGS=-I$(PWD)/../common

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f ../common/ptdump-common.o ../common/.ptdump-common.o.cmd
