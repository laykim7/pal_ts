#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xbff2abe1, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xde98c647, __VMLINUX_SYMBOL_STR(kobject_put) },
	{ 0xafd02d98, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0x74cf6b1f, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x51635c28, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0xe6da44a, __VMLINUX_SYMBOL_STR(set_normalized_timespec) },
	{ 0x784213a6, __VMLINUX_SYMBOL_STR(pv_lock_ops) },
	{ 0x391093cf, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0xc8b57c27, __VMLINUX_SYMBOL_STR(autoremove_wake_function) },
	{ 0x34184afe, __VMLINUX_SYMBOL_STR(current_kernel_time) },
	{ 0x8eac87bd, __VMLINUX_SYMBOL_STR(dma_set_mask) },
	{ 0xf6a4a37b, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0x60de9b89, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0xbc7676e3, __VMLINUX_SYMBOL_STR(x86_dma_fallback_dev) },
	{ 0x713611c7, __VMLINUX_SYMBOL_STR(pci_release_regions) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0xf432dd3d, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x5d41c87c, __VMLINUX_SYMBOL_STR(param_ops_charp) },
	{ 0x5b0fc56, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0xfb578fc5, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x912e9599, __VMLINUX_SYMBOL_STR(pci_iounmap) },
	{ 0xcb21ed87, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x8f64aa4, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x4bebaf06, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x2adb5b4f, __VMLINUX_SYMBOL_STR(kobject_init_and_add) },
	{ 0x7d52cf39, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x85fa44ee, __VMLINUX_SYMBOL_STR(__wake_up_locked) },
	{ 0x2072ee9b, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x5d595775, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0x4f9aaf56, __VMLINUX_SYMBOL_STR(module_put) },
	{ 0xc15136da, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0x78764f4e, __VMLINUX_SYMBOL_STR(pv_irq_ops) },
	{ 0xb2fd5ceb, __VMLINUX_SYMBOL_STR(__put_user_4) },
	{ 0x3bd1b1f6, __VMLINUX_SYMBOL_STR(msecs_to_jiffies) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{        0, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0x43261dca, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irq) },
	{ 0x83edb3ce, __VMLINUX_SYMBOL_STR(pv_cpu_ops) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0xcbccd081, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0xdf3deb56, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x29e1a175, __VMLINUX_SYMBOL_STR(pci_ioremap_bar) },
	{ 0x77811198, __VMLINUX_SYMBOL_STR(__dynamic_dev_dbg) },
	{ 0x9327f5ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0x34f22f94, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x1e047854, __VMLINUX_SYMBOL_STR(warn_slowpath_fmt) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xc0e90f8d, __VMLINUX_SYMBOL_STR(sysfs_create_groups) },
	{ 0x9696caf7, __VMLINUX_SYMBOL_STR(pci_request_regions) },
	{ 0x92c5f0e, __VMLINUX_SYMBOL_STR(pci_disable_msi) },
	{ 0x45e1ec8e, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0xd3719d59, __VMLINUX_SYMBOL_STR(paravirt_ticketlocks_enabled) },
	{ 0x634b9a3, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0xabea3957, __VMLINUX_SYMBOL_STR(sysfs_remove_groups) },
	{ 0x2b08fecb, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xfa66f77c, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x3ce8cc1a, __VMLINUX_SYMBOL_STR(dev_warn) },
	{ 0x48dfb368, __VMLINUX_SYMBOL_STR(pci_enable_msi_block) },
	{ 0x436c2179, __VMLINUX_SYMBOL_STR(iowrite32) },
	{ 0x9668ec0c, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x8d2e268b, __VMLINUX_SYMBOL_STR(param_ops_ulong) },
	{ 0x47c8baf4, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0xb4b7177e, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xa8c29637, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0x33e64620, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0x254326c4, __VMLINUX_SYMBOL_STR(try_module_get) },
	{ 0xe484e35f, __VMLINUX_SYMBOL_STR(ioread32) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010EEd00000007sv0000CAE2sd000071C0bc*sc*i*");

MODULE_INFO(srcversion, "58DEF24F9E3944F21A16A25");
