# 说明

在3A6000/3C6000处理器上配置memory model, 以实现CPU访存序约束控制。

* 关闭stfill，全地址W-W有序，W-R有序
* 开TSO, 全地址R-R有序、W-W有序、W-R有序

## 快速配置

`make config`编译配置程序：

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
