// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for Calixto AM64x Optima
 *
 * Copyright (C) 2026 Calixto Systems Pvt Ltd.
 * Based on board/ti/am64x/evm.c:
 * Copyright (C) 2020-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <efi_loader.h>
#include <asm/io.h>
#include <k3-ddrss.h>
#include <spl.h>
#include <fdt_support.h>
#include <asm/arch/hardware.h>
#include <asm/arch/k3-ddr.h>

DECLARE_GLOBAL_DATA_PTR;

/* TODO: replace TI SK GUIDs with Calixto-owned GUIDs */
struct efi_fw_image fw_images[] = {
        {
                .image_type_id = AM64X_SK_TIBOOT3_IMAGE_GUID,
                .fw_name = u"AM64X_SK_TIBOOT3",
                .image_index = 1,
        },
        {
                .image_type_id = AM64X_SK_SPL_IMAGE_GUID,
                .fw_name = u"AM64X_SK_SPL",
                .image_index = 2,
        },
        {
                .image_type_id = AM64X_SK_UBOOT_IMAGE_GUID,
                .fw_name = u"AM64X_SK_UBOOT",
                .image_index = 3,
        }
};

struct efi_capsule_update_info update_info = {
        .dfu_string = NULL,     /* no OSPI on optima; set eMMC layout later */
        .num_images = ARRAY_SIZE(fw_images),
        .images = fw_images,
};

#if defined(CONFIG_SPL_LOAD_FIT)
int board_fit_config_name_match(const char *name)
{
        if (!strcmp(name, "k3-am64x-r5-calixto-optima_2GB") ||
            !strcmp(name, "k3-am64x-calixto-optima_2GB"))
                return 0;

        return -1;
}
#endif

#if defined(CONFIG_XPL_BUILD)
#if CONFIG_IS_ENABLED(USB_STORAGE)
static int fixup_usb_boot(const void *fdt_blob)
{
        int ret = 0;

        switch (spl_boot_device()) {
        case BOOT_DEVICE_USB:
                ret = fdt_find_and_setprop((void *)fdt_blob,
                                           "/bus@f4000/cdns-usb@f900000/usb@f400000",
                                           "dr_mode", "host", 5, 0);
                if (ret)
                        printf("%s: fdt_find_and_setprop() failed:%d\n",
                               __func__, ret);
                fallthrough;
        default:
                break;
        }

        return ret;
}
#endif

void spl_perform_board_fixups(struct spl_image_info *spl_image)
{
        if (IS_ENABLED(CONFIG_K3_DDRSS)) {
                if (IS_ENABLED(CONFIG_K3_INLINE_ECC))
                        fixup_ddr_driver_for_ecc(spl_image);
        } else {
                fixup_memory_node(spl_image);
        }

#if CONFIG_IS_ENABLED(USB_STORAGE)
        fixup_usb_boot(spl_image->fdt_addr);
#endif
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
        /* TODO: derive eth1addr/eth2addr for ICSSG ports (MAC scheme pending) */
        return 0;
}
#endif

#define CTRLMMR_USB0_PHY_CTRL   0x43004008
#define CORE_VOLTAGE            0x80000000

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
        u32 val;

        /* Set USB PHY core voltage to 0.85V */
        val = readl(CTRLMMR_USB0_PHY_CTRL);
        val &= ~(CORE_VOLTAGE);
        writel(val, CTRLMMR_USB0_PHY_CTRL);

        /* Init DRAM size for R5/A53 SPL */
        dram_init_banksize();
}
#endif
