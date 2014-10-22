/*
 * debugfs_api.h
 *
 *  Created on: Oct 21, 2014
 *      Author: timkoma
 */

#ifndef DEBUGFS_API_H_
#define DEBUGFS_API_H_

#include <linux/device.h>
#include <linux/export.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

struct kobject *devm_kobject_create_and_add(struct device *dev, const char *name, struct kobject *parent);

void devm_kobject_put(struct device *dev, struct kobject *kobj);

struct dentry* devm_hello_create_dir(struct device *dev, const char *name, struct dentry* parent);

struct dentry* devm_hello_create_file(struct device *dev, const char *name, umode_t mode, struct dentry *parent, void *data, const struct file_operations* fops);

struct dentry* devm_hello_create_u32(struct device *dev, const char *name, umode_t mode, struct dentry *parent, u32* pu32);

#endif /* DEBUGFS_API_H_ */
