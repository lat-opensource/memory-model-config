# 说明

在3A6000/3C6000处理器上配置memory model, 以实现CPU访存序约束控制。

* 关闭stfill，全地址W-W有序，W-R有序
* 开TSO, 全地址R-R有序、W-W有序、W-R有序

## 使用方法

`make`编译config程序和内核模块（需要安装内核头文件）

`sudo insmod ./mem-model-config.ko`

dmesg检查打印下面日志
```
[256526.211779] create file /sys/kernel/loongarch_csr/stfill.
[256526.211781] create file /sys/kernel/loongarch_csr/tso_disabled
```

需要成功安装内核模块才能进行后续配置。

## 快速配置

使用：

- sudo ./config -h
- sudo ./config tso enable
- sudo ./config tso disable
- sudo ./config tso show
- sudo ./config stfill enable
- sudo ./config stfill disable
- sudo ./config stfill show

## 手动配置

### stfill

- 关 store fill:
echo 1  > /sys/kernel/loongarch_csr/stfill_disabled

- 开 store fill:
echo 0  > /sys/kernel/loongarch_csr/stfill_disabled

### tso

- 开 tso:
echo 1  > /sys/kernel/loongarch_csr/tso_disabled

- 关 tso:
echo 0  > /sys/kernel/loongarch_csr/tso_disabled
