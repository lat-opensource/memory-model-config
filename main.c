// SPDX-License-Identifier: GPL-2.0

#include "global.h"
#include <linux/kobject.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LoongArch memory model configuration via CSR 0x80");

extern int memory_model_init(void);
extern void memory_model_exit(void);

struct kobject* csr_kobj = NULL;

static int mem_model_config_init(void)
{
    int err1;
    csr_kobj = kobject_create_and_add("loongarch_csr", kernel_kobj);
    if (!csr_kobj) {
        printk("failed to create kobj, may exists already.\n");
        return -ENOMEM;
    }

    err1 = memory_model_init();
    if (err1)
        return err1;
    return 0;
}

static void mem_model_config_exit(void)
{
    memory_model_exit();
}

module_init(mem_model_config_init);
module_exit(mem_model_config_exit);
