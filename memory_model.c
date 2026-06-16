// SPDX-License-Identifier: GPL-2.0

#include "global.h"
#if defined(__has_include) && __has_include(<asm/loongarchregs.h>)
#include <asm/loongarchregs.h>
#else
#include <asm/loongarch.h>
#endif
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/sysfs.h>

static int  model = 0;
static void init_value(void);

int  memory_model_init(void);
void memory_model_exit(void);

// CSR 0x80 bit[30:28] encodings
//   bit[30]: store-store order
//   bit[29]: load-load order mode (0=full-address, 1=same-cacheline)
//   bit[28]: load-load order enable
#define MODEL_WEAK  0x0  // 000: no ordering
#define MODEL_STORE 0x4  // 100: store-store order only
#define MODEL_TSO   0x5  // 101: store-store + load-load order

static int get_bits(uint64_t value)
{
    return (value >> 28) & 0x7;
}

static void set_weak(void* failed)
{
    uint64_t tmp;

    tmp = read_csr_impctl1();
    tmp &= ~(0x7 << 28);
    write_csr_impctl1(tmp);

    if (tmp != read_csr_impctl1())
        *(int*)failed |= 1 << smp_processor_id();
    printk("loongarch: memory model weak  (gsconfig[%1d] %llx)\n",
        smp_processor_id(), tmp);
}

static void set_store(void* failed)
{
    uint64_t tmp;

    tmp = read_csr_impctl1();
    tmp &= ~(0x7 << 28);
    tmp |= MODEL_STORE << 28;
    write_csr_impctl1(tmp);

    if (tmp != read_csr_impctl1())
        *(int*)failed |= 1 << smp_processor_id();
    printk("loongarch: memory model store (gsconfig[%1d] %llx)\n",
        smp_processor_id(), tmp);
}

static void set_tso(void* failed)
{
    uint64_t tmp;

    tmp = read_csr_impctl1();
    tmp &= ~(0x7 << 28);
    tmp |= MODEL_TSO << 28;
    write_csr_impctl1(tmp);

    if (tmp != read_csr_impctl1())
        *(int*)failed |= 1 << smp_processor_id();
    printk("loongarch: memory model tso   (gsconfig[%1d] %llx)\n",
        smp_processor_id(), tmp);
}

static ssize_t model_show(struct kobject* kobj, struct kobj_attribute* attr,
    char* buf)
{
    return sprintf(buf, "%d\n", model);
}

static ssize_t model_store(struct kobject* kobj, struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    int ret = 0;

    switch (buf[0]) {
    case '0':
        on_each_cpu(set_weak, &ret, 1);
        if (!ret)
            model = 0;
        break;
    case '1':
        on_each_cpu(set_store, &ret, 1);
        if (!ret)
            model = 1;
        break;
    case '2':
        on_each_cpu(set_tso, &ret, 1);
        if (!ret)
            model = 2;
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
    switch (get_bits(tmp)) {
    case MODEL_WEAK:
        model = 0;
        break;
    case MODEL_STORE:
        model = 1;
        break;
    case MODEL_TSO:
        model = 2;
        break;
    default:
        model = 0;
        break;
    }
}

static struct kobj_attribute model_attribute = __ATTR(memory_model, 0660,
    model_show, model_store);

int memory_model_init(void)
{
    int error = 0;

    if (!csr_kobj) {
        printk("kobject is not initialized.\n");
        return -ENOMEM;
    }

    error = sysfs_create_file(csr_kobj, &model_attribute.attr);
    if (error) {
        printk("failed to create file /sys/kernel/loongarch_csr/memory_model\n");
    } else {
        init_value();
        printk("create file /sys/kernel/loongarch_csr/memory_model\n");
    }

    return error;
}

void memory_model_exit(void)
{
    kobject_put(csr_kobj);
}
