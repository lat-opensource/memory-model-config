// SPDX-License-Identifier: GPL-2.0

#include "global.h"
#include <asm/loongarchregs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/sysfs.h>

static int tso_disabled = 1;
static int prev_value;

//[30:28] 101
#define CSR_TSO 5 << 28

static int get_bits(uint64_t value)
{

    return (value >> 28) & 0x7;
}

static void disable_tso(void* failed)
{
    uint64_t tmp;

    tmp = read_csr_impctl1();
    // save
    prev_value = get_bits(tmp);
    // empty
    tmp &= ~(0x7 << 28);
    // set 0b101
    tmp |= CSR_STFILL;
    write_csr_impctl1(tmp);

    if (tmp != read_csr_impctl1())
        *(int*)failed |= 1 << smp_processor_id();
    printk("loongarch: gsconfig[%1d] %llx\n", smp_processor_id(), tmp);
}

static void enable_tso(void* failed)
{
    uint64_t tmp;

    tmp = read_csr_impctl1();
    // empty
    tmp &= ~(0x7 << 28);
    // restore
    tmp |= prev_value << 28;

    write_csr_impctl1(tmp);

    if (tmp != read_csr_impctl1())
        *(int*)failed |= 1 << smp_processor_id();
    printk("loongarch: gsconfig[%1d] %llx\n", smp_processor_id(), tmp);
}

static ssize_t tso_show(struct kobject* kobj, struct kobj_attribute* attr,
    char* buf)
{
    return sprintf(buf, "%d\n", tso_disabled);
}

static ssize_t tso_store(struct kobject* kobj, struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    int ret = 0;

    switch (buf[0]) {
    case '0':
        on_each_cpu(enable_tso, &ret, 1);
        if (ret) {
            printk("loongarch: failed to enable store fill %x\n", ret);
        } else {
            tso_disabled = 0;
            printk("loongarch: store fill is now enabled\n");
        }
        break;
    case '1':
        on_each_cpu(disable_tso, &ret, 1);
        if (ret) {
            printk("loongarch: failed to disable store fill %x\n", ret);
        } else {
            tso_disabled = 1;
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
    // save
    prev_value = get_bits(tmp);
    tso_disabled = (prev_value != 0b101);
}

static struct kobj_attribute tso_attribute = __ATTR(tso_disabled, 0660,
    tso_show, tso_store);

int tso_init(void)
{
    int error = 0;

    if (!csr_kobj) {
        printk("kobject is not initialized.\n");
        return -ENOMEM;
    }

    error = sysfs_create_file(csr_kobj, &tso_attribute.attr);
    if (error) {
        init_value();
        printk("failed to create the file in /sys/kernel/loongarch_csr\n");
    } else {
        printk("create file /sys/kernel/loongarch_csr/tso_disabled\n");
    }

    return error;
}
void tso_exit(void)
{
    kobject_put(csr_kobj);
}
