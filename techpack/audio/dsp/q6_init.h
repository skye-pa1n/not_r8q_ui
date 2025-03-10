/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 */

#ifndef __Q6_INIT_H__
#define __Q6_INIT_H__
int adsp_err_init(void);
int adm_init(void);
int afe_init(void);
int q6asm_init(void);
int q6lsm_init(void);
int voice_init(void);
int audio_cal_init(void);
int qcore_init(void);
int rtac_init(void);
int msm_audio_ion_init(void);
int avtimer_init(void);
#ifdef CONFIG_MSM_MDF
int msm_mdf_init(void);
void msm_mdf_exit(void);
#else
static inline int msm_mdf_init(void)
{
	return 0;
}

static inline void msm_mdf_exit(void)
{
	return;
}
#endif
#ifdef CONFIG_XT_LOGGING
int spk_params_init(void);
void spk_params_exit(void);
#else
static inline int spk_params_init(void)
{
	return 0;
}
static inline void spk_params_exit(void)
{
}
#endif

#ifdef CONFIG_SEC_SND_ADAPTATION
int sec_soc_platform_init(void);
void sec_soc_platform_exit(void);
#endif /* CONFIG_SEC_SND_ADAPTATION */

void avtimer_exit(void);
void msm_audio_ion_exit(void);
void rtac_exit(void);
void core_exit(void);
void audio_cal_exit(void);
void voice_exit(void);
void q6lsm_exit(void);
void q6asm_exit(void);
void afe_exit(void);
void adm_exit(void);
void adsp_err_exit(void);
#if IS_ENABLED(CONFIG_WCD9XXX_CODEC_CORE)
int audio_slimslave_init(void);
void audio_slimslave_exit(void);
#else
static inline int audio_slimslave_init(void)
{
	return 0;
};
static inline void audio_slimslave_exit(void)
{
};
#endif
#ifdef CONFIG_VOICE_MHI
int voice_mhi_init(void);
void voice_mhi_exit(void);
#else
static inline int voice_mhi_init(void)
{
	return 0;
}

static inline void voice_mhi_exit(void)
{
	return;
}
#endif

#ifdef CONFIG_DIGITAL_CDC_RSC_MGR
void digital_cdc_rsc_mgr_init(void);
void digital_cdc_rsc_mgr_exit(void);
#else
static inline void digital_cdc_rsc_mgr_init(void)
{
}

static inline void digital_cdc_rsc_mgr_exit(void)
{
}
#endif /* CONFIG_DIGITAL_CDC_RSC_MGR */

#endif

