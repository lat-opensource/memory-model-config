// SPDX-License-Identifier: GPL-2.0

#include "global.h"
#include <linux/kobject.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

extern int tso_init(void);
extern int stfill_init(void);
extern void stfill_exit(void);
extern void tso_exit(void);

struct kobject* csr_kobj = NULL;

static int mem_model_config_init(void)
{
    int err1;
    int err2;
    csr_kobj = kobject_create_and_add("loongarch_csr", kernel_kobj);
    if (!csr_kobj) {
        printk("failed to create kobj, may exists already.\n");
        return -ENOMEM;
    }

    err1 = stfill_init();
    if (err1)
        return err1;
    err2 = tso_init();
    if (err2)
        return err2;
    return 0;
}

static void mem_model_config_exit(void)
{
    stfill_exit();
    tso_exit();
}

module_init(mem_model_config_init);
module_exit(mem_model_config_exit);
