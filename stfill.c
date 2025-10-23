// SPDX-License-Identifier: GPL-2.0

#include "global.h"
#include <asm/loongarchregs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/sysfs.h>

static int stfill;

static void disable_stfill(void* failed)
{
    uint64_t tmp;

    tmp = read_csr_impctl1();
    tmp &= ~CSR_STFILL;
    write_csr_impctl1(tmp);

    if (tmp != read_csr_impctl1())
        *(int*)failed |= 1 << smp_processor_id();
    printk("loongarch: gsconfig[%1d] %llx\n", smp_processor_id(), tmp);
}

static void enable_stfill(void* failed)
{
    uint64_t tmp;

    tmp = read_csr_impctl1();
    tmp |= CSR_STFILL;
    write_csr_impctl1(tmp);

    if (tmp != read_csr_impctl1())
        *(int*)failed |= 1 << smp_processor_id();
    printk("loongarch: gsconfig[%1d] %llx\n", smp_processor_id(), tmp);
}

static ssize_t stfill_show(struct kobject* kobj, struct kobj_attribute* attr,
    char* buf)
{
    return sprintf(buf, "%d\n", stfill);
}

static ssize_t stfill_store(struct kobject* kobj, struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    int ret = 0;

    switch (buf[0]) {
    case '0':
        on_each_cpu(enable_stfill, &ret, 1);
        if (ret) {
            printk("loongarch: failed to enable store fill %x\n", ret);
        } else {
            stfill = 1;
            printk("loongarch: store fill is now enabled\n");
        }
        break;
    case '1':
        on_each_cpu(disable_stfill, &ret, 1);
        if (ret) {
            printk("loongarch: failed to disable store fill %x\n", ret);
        } else {
            stfill = 0;
            printk("loongarch: store fill is now disabled\n");
        }
        break;
    default:
        ret = -EINVAL;
    }

    return (ret == 0) ? count : -EPERM;
}
static void init_value(void)
{
    int tmp;
    tmp = read_csr_impctl1();
    stfill = (tmp & CSR_STFILL) >> (CSR_STFILL_SHIFT);
}

static struct kobj_attribute stfill_attribute = __ATTR(stfill, 0660,
    stfill_show, stfill_store);

int stfill_init(void)
{
    int error = 0;

    if (!csr_kobj) {
        printk("kobject is not initialized.\n");
        return -ENOMEM;
    }

    error = sysfs_create_file(csr_kobj, &stfill_attribute.attr);
    if (error) {
        printk("failed to create the file in /sys/kernel/loongarch_csr.\n");
    } else {
        init_value();
        printk("create file /sys/kernel/loongarch_csr/stfill.\n");
    }

    return error;
}

void stfill_exit(void)
{
    kobject_put(csr_kobj);
}
