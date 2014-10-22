#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include "debugfs_api.h"

MODULE_LICENSE("GPL");

static void devm_kobject_release(struct device *dev, void *res)
{
	kobject_put(*(struct kobject **)res);
}

struct kobject *devm_kobject_create_and_add(struct device *dev, const char *name, struct kobject *parent)
{
	struct kobject **ptr, *kobj;

	ptr = devres_alloc(devm_kobject_release, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return ERR_PTR(-ENOMEM);

	kobj = kobject_create_and_add("kobject_example", &dev->kobj);
	if (kobj) {
		*ptr = kobj;
		devres_add(dev, ptr);
	}
	else {
		devres_free(ptr);
	}

	return kobj;
}
EXPORT_SYMBOL(devm_kobject_create_and_add);

static int devm_kobject_match(struct device *dev, void *res, void *data)
{
	struct kobject **c = res;
	if (!c || !*c) {
		WARN_ON(!c || !*c);
		return 0;
	}
	return *c == data;
}

void devm_kobject_put(struct device *dev, struct kobject *kobj)
{
	int ret;

	ret = devres_release(dev, devm_kobject_release, devm_kobject_match, kobj);

	WARN_ON(ret);
}
EXPORT_SYMBOL(devm_kobject_put);

static void devm_hello_release(struct device *dev, void *res)
{
	printk(KERN_ALERT "hello-debugfs, devm_hello_release\n");
	debugfs_remove(res);
}

struct dentry* devm_hello_create_u32(struct device *dev, const char *name, umode_t mode, struct dentry *parent, u32 *pvalue)
{
	struct dentry *pu32 = NULL;\
	void *ptr = NULL;

	if(!pvalue || !name || name[0] == '\0')
		return NULL;

	ptr = devres_alloc(devm_hello_release, 0, GFP_KERNEL);
	if (!ptr)
		return NULL;

	pu32 = debugfs_create_u32(name, mode, parent, pvalue);
	if (pu32) {
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return pu32;
}
EXPORT_SYMBOL(devm_hello_create_u32);

struct dentry* devm_hello_create_file(
		struct device *dev,
		const char *name,
		umode_t mode,
		struct dentry *parent,
		void *data,
		const struct file_operations* fops)
{
	struct dentry *pfile = NULL;
	void *ptr = NULL;

	if(!fops || !name || name[0] == '\0')
		return NULL;

	ptr = devres_alloc(devm_hello_release, 0, GFP_KERNEL);
	if (!ptr)
		return NULL;

	pfile = debugfs_create_file(name, mode, parent, data, fops);
	if (pfile) {
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return pfile;
}
EXPORT_SYMBOL(devm_hello_create_file);

struct dentry* devm_hello_create_dir(struct device *dev, const char *name, struct dentry *parent)
{

	struct dentry *dir = NULL;
	void *ptr = NULL;

	if(!name || name[0] == '\0')
		return NULL;

	ptr = devres_alloc(devm_hello_release, 0, GFP_KERNEL);
	if (!ptr)
		return NULL;

	dir = debugfs_create_dir(name, parent);
	if (dir) {
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return dir;
}
EXPORT_SYMBOL(devm_hello_create_dir);
