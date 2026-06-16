# 说明

在3A6000/3C6000处理器上配置memory model, 以实现CPU访存序约束控制。

三种模式（写入 CSR 0x80 bit[30:28]）：
* weak:  0b000, 无序
* store: 0b100, store-store 有序
* tso:   0b101, store-store 有序 + load-load 有序

## 使用方法

`make` 编译 config 程序和内核模块（需要安装内核头文件）

`sudo insmod ./mem-model-config.ko`

dmesg 检查打印下面日志
```
[256526.211779] create file /sys/kernel/loongarch_csr/memory_model
```

需要成功安装内核模块才能进行后续配置。

## 交互式配置

```
sudo ./config
```

进入交互界面，显示当前状态，按键选择：
- `0` — 切换到 weak
- `1` — 切换到 store  
- `2` — 切换到 tso
- `q` — 退出

## 手动配置

- 弱序:
echo 0  > /sys/kernel/loongarch_csr/memory_model

- store:
echo 1  > /sys/kernel/loongarch_csr/memory_model

- tso:
echo 2  > /sys/kernel/loongarch_csr/memory_model
