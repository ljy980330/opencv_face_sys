KERN_DIR = /home/Linux_kernel/4.4.y/linux

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	rm -rf *.o
	make -C $(KERN_DIR) M=`pwd` modules clean

obj-m	+= hello_driver.o

