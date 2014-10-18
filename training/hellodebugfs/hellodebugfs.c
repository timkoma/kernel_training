#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/debugfs.h>

MODULE_AUTHOR("Marek Timko");
MODULE_DESCRIPTION("MT hello bus adapter");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:hello-debugfs");

struct hello_dbgfs{
	struct dentry *directory;
	struct dentry *counter;
	struct dentry *file;
};

struct hello_dev{
	struct hello_dbgfs dbgfs;
};

static int debugfs_open(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "hello-debugfs, debugfs_open\n");
	return 0;
}

static ssize_t debugfs_read(
	struct file *file, /* file descriptor */
	char __user *user_buf,  /* user buffer */
	size_t length,          /* length of buffer */
	loff_t *offset          /* offset in the file */)
{
	printk(KERN_ALERT "hello-debugfs, debugfs_read\n");
	return 0;
}

static ssize_t debugfs_write(struct file *file,
	const char __user *user_buf,
	size_t length,
	loff_t *offset)
{
	printk(KERN_ALERT "hello-debugfs, debugfs_write\n");
	return 0;
}

int hello_create_object(struct device *dev,void *data)
{
	static const struct file_operations debugfs_file_ops = {
		.owner = THIS_MODULE,
		.open = debugfs_open,
		.read = debugfs_read,
		.write = debugfs_write,
	};
	u32 u32counter;
	struct hello_dev* hellodev = dev_get_drvdata(dev);
	if(!hellodev)
		return -1;

	hellodev->dbgfs.directory = debugfs_create_dir("hello-debugfs", NULL);
	hellodev->dbgfs.counter = debugfs_create_u32("counter", 0666, hellodev->dbgfs.directory, &u32counter);
	hellodev->dbgfs.file = debugfs_create_file("file-debugfs", 0666, hellodev->dbgfs.directory, NULL, &debugfs_file_ops);
	return 0;
}

static void devm_hello_release(struct device *dev, void *res)
{
	struct hello_dev* hellodev = dev_get_drvdata(dev);

	printk(KERN_ALERT "hello-debugfs, devm_hello_release\n");

	if(!hellodev)
		return;

	if(hellodev->dbgfs.file){
		debugfs_remove(hellodev->dbgfs.file);
		hellodev->dbgfs.file = NULL;
	}
	if(hellodev->dbgfs.counter){
		debugfs_remove(hellodev->dbgfs.counter);
		hellodev->dbgfs.counter = NULL;
	}
	if(hellodev->dbgfs.directory){
		debugfs_remove(hellodev->dbgfs.directory);
		hellodev->dbgfs.directory = NULL;
	}
}

int devm_hello_create_object(struct device *dev,void *data)
{
	struct hello_dbgfs **ptr;
	int ret;
	struct hello_dev* hellodev = dev_get_drvdata(dev);

	ptr = devres_alloc(devm_hello_release, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return -ENOMEM;

	ret = hello_create_object(dev, data);
	if (ret == 0) {
		*ptr = &hellodev->dbgfs;
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return 0;
}

static int hello_remove(struct platform_device *pdev)
{
       printk(KERN_ALERT "hello-debugfs, remove\n");
       kfree(platform_get_drvdata(pdev));
       return 0;
}

static int hello_probe(struct platform_device *pdev)
{
	struct hello_dev *data;
	int ret;

	printk(KERN_ALERT "hellow-debugfs, probe.....\n");
	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	platform_set_drvdata(pdev, data);

	ret = devm_hello_create_object(&pdev->dev, NULL);
	printk(KERN_ALERT "hellow-debugfs, devm_hello_create_object: %d\n", ret);
	return 0;
}

static const struct of_device_id hello_of_match[] = {
       {
               .compatible = "gl,hello-debugfs",
       },
       { },
};
MODULE_DEVICE_TABLE(of, hello_of_match);

static struct platform_driver hello_debugfs_driver = {
       .probe          = hello_probe,
       .remove         = hello_remove,
       .driver         = {
               .name   = "hello-debugfs",
               .owner  = THIS_MODULE,
               .pm     = NULL,
               .of_match_table = of_match_ptr(hello_of_match),
       },
};

module_platform_driver(hello_debugfs_driver);

