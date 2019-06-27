#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

#define DEVICEMAJOR 0 //主设备号
#define DEVICEMINOR 0 //次设备号
#define MINORNUM 2 //次设备个数,可以根据需要进行修改
#define DEVICENAME "helloTest"
#define LEN (MINORNUM * sizeof(struct reg_devs))
#define DEVICE_SIZE 2048

static int numdev_major = DEVICEMAJOR,numdev_minor = DEVICEMINOR;

static struct class *hellodrv_class;
static struct device *hellodrv_dev[MINORNUM];

struct reg_devs
{
	char *data;
	unsigned long size;

	struct cdev cdev;
};
struct reg_devs *hello_dev;


static int hello_drv_open(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hello_drv_open\n");
	
	return 0;
}

static ssize_t hello_drv_read(struct file *fd, char __user *buf, size_t count, loff_t *ppos)
{
	printk(KERN_EMERG "hello_drv_read\n");

	return 0;
}

static ssize_t hello_drv_write(struct file *fd, const char __user *buf, size_t count, loff_t *ppos)
{
	printk(KERN_EMERG "hello_drv_write\n");

	return 0;
}

static struct file_operations hello_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   hello_drv_open,
    .read 	=   hello_drv_read,
    .write	=   hello_drv_write,
};

static void reg_init_cdev(struct reg_devs *dev,int inode){
	int ret;
	dev_t dev_num;
	
	dev_num = MKDEV(numdev_major,numdev_minor + inode);
	
	cdev_init(&dev->cdev, &hello_fops);
//	dev->cdev.owner = THIS_MODULE;
//	dev->cdev.ops = &my_ops;

	ret = cdev_add(&dev->cdev, dev_num, 1);
	if(ret){
		printk(KERN_EMERG "cdev_add %d is failed!\n",ret);
	}
	else{
		printk(KERN_EMERG "cdev_add dev[%d] is %d OK!\n",inode,ret);
	}
}

static int hello_drv_init(void)
{
	int i;
	dev_t dev_num;

	printk(KERN_EMERG "%s init....\n", DEVICENAME);

	//动态申请字符类设备号
	if(numdev_major == 0)
	{
		alloc_chrdev_region(&dev_num, numdev_minor, MINORNUM, DEVICENAME);
		numdev_major = MAJOR(dev_num);
		printk(KERN_EMERG "alloc_chrdev_region numdev_major is %d!\n", numdev_major);	
	}
	else
	{
		printk(KERN_EMERG "This driver is dynamically applying for character class device numbers.\n");
		return -1;
	}

	//创建设备结点类
	hellodrv_class = class_create(THIS_MODULE, DEVICENAME);

	//分配内存空间
	hello_dev = (struct reg_devs *)kmalloc(LEN, GFP_KERNEL);
	if(hello_dev == 0)
	{
		unregister_chrdev_region(MKDEV(numdev_major,numdev_minor), MINORNUM);
		printk(KERN_EMERG "kamlloc is failed!\n");
		return -ENOMEM;
	}

	//清空内存空间
	memset(hello_dev,0,LEN);
	for(i=0; i<MINORNUM; i++)
	{
		//创建设备结点
		hellodrv_dev[i] = device_create(hellodrv_class, NULL, MKDEV(numdev_major, numdev_minor+i), NULL, DEVICENAME"%d", i);
		
		hello_dev[i].data = (char *)kmalloc(DEVICE_SIZE, GFP_KERNEL);
		memset(hello_dev[i].data, 0, DEVICE_SIZE);
		reg_init_cdev(&hello_dev[i], i);
	}

	return 0;
}

static void hello_drv_exit(void)
{
	int i;

	for(i=0; i<MINORNUM; i++)
	{
		device_unregister(hellodrv_dev[i]);
		cdev_del(&hello_dev[i].cdev);
		printk(KERN_EMERG "cdev_del dev[%d] OK!\n",i);
	}

	class_destroy(hellodrv_class);
	unregister_chrdev_region(MKDEV(numdev_major,numdev_minor), MINORNUM);
	printk(KERN_EMERG "%s exit....\n", DEVICENAME);
}

module_init(hello_drv_init);
module_exit(hello_drv_exit);

MODULE_LICENSE("GPL");
