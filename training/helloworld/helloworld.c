#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

MODULE_AUTHOR("Marek Timko");
MODULE_DESCRIPTION("MT hello bus adapter");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:helloworld");

static int hello_remove(struct platform_device *pdev)
{
       printk(KERN_ALERT "helloworld, remove\n");
       return 0;
}

static int hello_probe(struct platform_device *pdev)
{
       printk(KERN_ALERT "helloworld, probe module.....\n");
       return 0;
}

static const struct of_device_id hello_of_match[] = {
       {
               .compatible = "gl,helloworld",
       },
       { },
};
MODULE_DEVICE_TABLE(of, hello_of_match);

static struct platform_driver helloworld_driver = {
       .probe          = hello_probe,
       .remove         = hello_remove,
       .driver         = {
               .name   = "helloworld",
               .owner  = THIS_MODULE,
               .pm     = NULL,
               .of_match_table = of_match_ptr(hello_of_match),
       },
};

module_platform_driver(hellowolrd_driver);

