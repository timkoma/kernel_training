#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include "debugfs_api.h"

MODULE_AUTHOR("Marek Timko");
MODULE_DESCRIPTION("MT hello bus adapter");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:hello-debugfs");

#define DBG_FILE_DATA_LEN	32

struct hello_dbgfs{
	struct dentry *directory;
	struct dentry *counter;
	struct dentry *file;
};

struct hello_atributes {
	int foo;
	int baz;
	int bar;
};

struct device_data{
	struct hello_dbgfs dbgfs;
	u32 counter;
	char dbgfs_file_data[DBG_FILE_DATA_LEN];
	//struct hello_atributes sysfs_attr;
};

struct hello_atributes *g_attr = NULL;

/*
 * The "foo" file where a static variable is read from and written to.
 */
static ssize_t foo_show(struct device *dev, struct device_attribute *d_attr, char *buf)
{
	//struct device_data *data = dev_get_drvdata(dev);

	printk(KERN_ALERT "hello, show foo\n");
	//return sprintf(buf, "%d hello\n", data->sysfs_attr.foo);//[  125.608755] Unable to handle kernel paging request at virtual address 00100130
	return sprintf(buf, "%d hello\n", g_attr->foo);//[  125.608755] Unable to handle kernel paging request at virtual address 00100130
}

static ssize_t foo_store(struct device *dev, struct device_attribute *d_attr, const char *buf, size_t count)
{
	//struct device_data *data = dev_get_drvdata(dev);

	printk(KERN_ALERT "hello, store foo\n");
	sscanf(buf, "%du", &g_attr->foo);
	return count;
}

static DEVICE_ATTR(foo, 0666, foo_show, foo_store);

/*
 * More complex function where we determine which variable is being accessed by
 * looking at the attribute for the "baz" and "bar" files.
 */
static ssize_t b_show(struct device *dev, struct device_attribute *d_attr, char *buf)
{
	//struct device_data *data = dev_get_drvdata(dev);
	int var = 0;

	printk(KERN_ALERT "hello, show location:%s\n",d_attr->attr.name);
	if (strcmp(d_attr->attr.name, "baz") == 0)
		var = g_attr->baz;
	else
		var = g_attr->bar;
	return sprintf(buf, "%d\n", var);
}

static ssize_t b_store(struct device *dev, struct device_attribute *d_attr, const char *buf, size_t count)
{
	//struct device_data *data = dev_get_drvdata(dev);
	int var = 0;

	sscanf(buf, "%du", &var);
	printk(KERN_ALERT "hello, store location:%s\n",d_attr->attr.name);
	if (strcmp(d_attr->attr.name, "baz") == 0)
		g_attr->baz = var;
	else
		g_attr->bar = var;
	return count;
}

static DEVICE_ATTR(baz, 0666, b_show, b_store);
static DEVICE_ATTR(bar, 0666, b_show, b_store);

static struct attribute *attrs[] = {
	&dev_attr_foo.attr,
	&dev_attr_baz.attr,
	&dev_attr_bar.attr,
	NULL,
};

static struct attribute_group attr_group = {
       .attrs = attrs,
};

//static struct kobject *example_kobj;

static int debugfs_open(struct inode *inode, struct file *pfile)
{
	pfile->private_data = inode ? inode->i_private : 0;
	printk(KERN_ALERT "hello-debugfs, debugfs_open private: %p\n", pfile->private_data);
	return 0;
}

static ssize_t debugfs_read(
	struct file *pfile, /* file descriptor */
	char __user *user_buf,  /* user buffer */
	size_t length,          /* length of buffer */
	loff_t *offset          /* offset in the file */)
{
	struct device_data *data = dev_get_drvdata((struct device*)pfile->private_data);

	printk(KERN_ALERT "hello-debugfs, debugfs_read\n");

	if (!data || *offset >= DBG_FILE_DATA_LEN)
		return 0;
	if (*offset + length > DBG_FILE_DATA_LEN)
		length = DBG_FILE_DATA_LEN - *offset;
	if (copy_to_user(user_buf, data->dbgfs_file_data + *offset, length))
		return -EFAULT;
	*offset += length;
	return length;
}

static ssize_t debugfs_write(struct file *pfile,
	const char __user *user_buf,
	size_t length,
	loff_t *offset)
{
	struct device_data *data = dev_get_drvdata((struct device*)pfile->private_data);

	printk(KERN_ALERT "hello-debugfs, debugfs_write\n");

    if (!data || *offset >= DBG_FILE_DATA_LEN)
		return 0;
    if (*offset + length > DBG_FILE_DATA_LEN)
		length = DBG_FILE_DATA_LEN - *offset;
    if (copy_from_user(data->dbgfs_file_data + *offset, user_buf, length))
            return -EFAULT;
    *offset += length;
    return length;
}

int hello_create_debugfs_objects(struct device *dev)
{
	static const struct file_operations debugfs_file_ops = {
		.owner = THIS_MODULE,
		.open = debugfs_open,
		.read = debugfs_read,
		.write = debugfs_write
	};
	struct device_data* dev_data = dev_get_drvdata(dev);
	if(!dev_data)
		return -1;

	dev_data->dbgfs.directory = devm_hello_create_dir(dev, "hello-debugfs", NULL);
	dev_data->dbgfs.counter = devm_hello_create_u32(dev, "counter", 0666, dev_data->dbgfs.directory, &dev_data->counter);
	dev_data->dbgfs.file = devm_hello_create_file(dev, "file-debugfs", 0666, dev_data->dbgfs.directory, dev, &debugfs_file_ops);
	return 0;
}

static int hello_remove(struct platform_device *pdev)
{
	printk(KERN_ALERT "hello-debugfs, remove\n");

	//sysfs_remove_group(example_kobj,&attr_group);

	return 0;
}

static int hello_probe(struct platform_device *pdev)
{
	struct device_data *data = NULL;
	int ret = 0;
	struct kobject *kobj = NULL;

	printk(KERN_ALERT "hello-debugfs, probe.....\n");

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->counter = 123;
	platform_set_drvdata(pdev, data);

	ret = hello_create_debugfs_objects(&pdev->dev);
	printk(KERN_ALERT "hello-debugfs, devm_hello_create_object: %d\n", ret);
	if(ret)
		return ret;

	g_attr = devm_kzalloc(&pdev->dev, sizeof(struct hello_atributes), GFP_KERNEL);

	kobj = devm_kobject_create_and_add(&pdev->dev, "kobject_example", &pdev->dev.kobj);
	printk(KERN_ALERT "hello-debugfs, devm_kobject_create_and_add: %d\n", ret);
	if (!kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	ret = sysfs_create_group(kobj, &attr_group);
	printk(KERN_ALERT "hello-debugfs, sysfs_create_group: %d\n", ret);
	if (ret) {
		devm_kobject_put(&pdev->dev, kobj);
	}

	return ret;
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

