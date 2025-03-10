// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017, 2019-2020 The Linux Foundation. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include "q6_init.h"

static int __init audio_q6_init(void)
{
	adsp_err_init();
	audio_cal_init();
	rtac_init();
	adm_init();
	afe_init();
	spk_params_init();
	q6asm_init();
	q6lsm_init();
	voice_init();
	qcore_init();
	msm_audio_ion_init();
	audio_slimslave_init();
	avtimer_init();
	msm_mdf_init();
	voice_mhi_init();
	digital_cdc_rsc_mgr_init();
#ifdef CONFIG_SEC_SND_ADAPTATION
	sec_soc_platform_init();
#endif /* CONFIG_SEC_SND_ADAPTATION */

	return 0;
}

static void __exit audio_q6_exit(void)
{
#ifdef CONFIG_SEC_SND_ADAPTATION
	sec_soc_platform_exit();
#endif /* CONFIG_SEC_SND_ADAPTATION */
	digital_cdc_rsc_mgr_exit();
	msm_mdf_exit();
	avtimer_exit();
	audio_slimslave_exit();
	msm_audio_ion_exit();
	core_exit();
	voice_exit();
	q6lsm_exit();
	q6asm_exit();
	afe_exit();
	spk_params_exit();
	adm_exit();
	rtac_exit();
	audio_cal_exit();
	adsp_err_exit();
	voice_mhi_exit();
}

module_init(audio_q6_init);
module_exit(audio_q6_exit);
MODULE_DESCRIPTION("Q6 module");
MODULE_LICENSE("GPL v2");
