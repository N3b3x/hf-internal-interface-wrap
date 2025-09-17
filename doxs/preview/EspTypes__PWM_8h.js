var EspTypes__PWM_8h =
[
    [ "hf_pwm_eviction_request_t", "structhf__pwm__eviction__request__t.html", "structhf__pwm__eviction__request__t" ],
    [ "hf_pwm_unit_config_t", "structhf__pwm__unit__config__t.html", "structhf__pwm__unit__config__t" ],
    [ "hf_pwm_channel_status_t", "structhf__pwm__channel__status__t.html", "structhf__pwm__channel__status__t" ],
    [ "hf_pwm_timing_config_t", "structhf__pwm__timing__config__t.html", "structhf__pwm__timing__config__t" ],
    [ "hf_pwm_channel_config_t", "structhf__pwm__channel__config__t.html", "structhf__pwm__channel__config__t" ],
    [ "hf_pwm_fade_config_t", "structhf__pwm__fade__config__t.html", "structhf__pwm__fade__config__t" ],
    [ "hf_pwm_capabilities_t", "structhf__pwm__capabilities__t.html", "structhf__pwm__capabilities__t" ],
    [ "hf_pwm_eviction_callback_t", "EspTypes__PWM_8h.html#aa88988c1a4ef6005d3f20421ecba42ed", null ],
    [ "hf_pwm_channel_priority_t", "EspTypes__PWM_8h.html#ad8fbde8152ecafefd714f3a7eb8344c1", [
      [ "PRIORITY_LOW", "EspTypes__PWM_8h.html#ad8fbde8152ecafefd714f3a7eb8344c1a4a2343657e145cb3dab4f445d2c29b1f", null ],
      [ "PRIORITY_NORMAL", "EspTypes__PWM_8h.html#ad8fbde8152ecafefd714f3a7eb8344c1a860a24d0e7b2fcadbbb258aad2708ad7", null ],
      [ "PRIORITY_HIGH", "EspTypes__PWM_8h.html#ad8fbde8152ecafefd714f3a7eb8344c1a47a10a0fba7f10a751e6ea38f3d6a1e3", null ],
      [ "PRIORITY_CRITICAL", "EspTypes__PWM_8h.html#ad8fbde8152ecafefd714f3a7eb8344c1a5f9223804ac004dc2fad18afc18a3951", null ]
    ] ],
    [ "hf_pwm_clock_source_t", "EspTypes__PWM_8h.html#ad387f5c3904a1913133dc217dacf7a82", [
      [ "HF_PWM_CLK_SRC_DEFAULT", "EspTypes__PWM_8h.html#ad387f5c3904a1913133dc217dacf7a82a7d16cfde99618b306f8de171107ba44c", null ],
      [ "HF_PWM_CLK_SRC_APB", "EspTypes__PWM_8h.html#ad387f5c3904a1913133dc217dacf7a82a2fb4f97b7c1b78586a5f9f8365272822", null ],
      [ "HF_PWM_CLK_SRC_XTAL", "EspTypes__PWM_8h.html#ad387f5c3904a1913133dc217dacf7a82a5e089a1c8943ad8da53d90cc891103ea", null ],
      [ "HF_PWM_CLK_SRC_RC_FAST", "EspTypes__PWM_8h.html#ad387f5c3904a1913133dc217dacf7a82a77f2a98c784c9990a858bf08b5e25f7d", null ]
    ] ],
    [ "hf_pwm_eviction_decision_t", "EspTypes__PWM_8h.html#afe53e31a856c24db0547fcf306e1b312", [
      [ "DENY_EVICTION", "EspTypes__PWM_8h.html#afe53e31a856c24db0547fcf306e1b312a05b624748cf8bbb2413a05507e813b2a", null ],
      [ "ALLOW_EVICTION", "EspTypes__PWM_8h.html#afe53e31a856c24db0547fcf306e1b312aa12d41f24d6d7c033f8b261d45fada97", null ],
      [ "SUGGEST_ALTERNATIVE", "EspTypes__PWM_8h.html#afe53e31a856c24db0547fcf306e1b312a4d410384cd4aae825bad97f4d13cf7b5", null ]
    ] ],
    [ "hf_pwm_eviction_policy_t", "EspTypes__PWM_8h.html#ac87a4c9f233c95c6036c956aa21fdf72", [
      [ "STRICT_NO_EVICTION", "EspTypes__PWM_8h.html#ac87a4c9f233c95c6036c956aa21fdf72a149573f7fc27bf6e07f0d47846625809", null ],
      [ "ALLOW_EVICTION_WITH_CONSENT", "EspTypes__PWM_8h.html#ac87a4c9f233c95c6036c956aa21fdf72a8baa33d4602f9cbcdaa7ef13dc035a72", null ],
      [ "ALLOW_EVICTION_NON_CRITICAL", "EspTypes__PWM_8h.html#ac87a4c9f233c95c6036c956aa21fdf72adf490f88b592696d2f33a4e4d137e0a0", null ],
      [ "FORCE_EVICTION", "EspTypes__PWM_8h.html#ac87a4c9f233c95c6036c956aa21fdf72a426a3594d7497ed09e36a62c3b1cc5d7", null ]
    ] ],
    [ "hf_pwm_fade_mode_t", "EspTypes__PWM_8h.html#aa1c95e596c136b279f69f137cd8e8370", [
      [ "HF_PWM_FADE_NO_WAIT", "EspTypes__PWM_8h.html#aa1c95e596c136b279f69f137cd8e8370ace9524a4d73c80904ccf4648faec815f", null ],
      [ "HF_PWM_FADE_WAIT_DONE", "EspTypes__PWM_8h.html#aa1c95e596c136b279f69f137cd8e8370a4de1dd10ebbfcfceeedcf0ece0632d1a", null ]
    ] ],
    [ "hf_pwm_intr_type_t", "EspTypes__PWM_8h.html#a58935b1b2788129ca9d313445ab9e5b4", [
      [ "HF_PWM_INTR_DISABLE", "EspTypes__PWM_8h.html#a58935b1b2788129ca9d313445ab9e5b4a911e18551ea86207c7c63892582c354a", null ],
      [ "HF_PWM_INTR_FADE_END", "EspTypes__PWM_8h.html#a58935b1b2788129ca9d313445ab9e5b4ae221ff47d56876986a3e7d491b04486b", null ]
    ] ],
    [ "hf_pwm_mode_t", "EspTypes__PWM_8h.html#aa74b1b34478ef0fde22cd029366ab3fa", [
      [ "HF_PWM_MODE_BASIC", "EspTypes__PWM_8h.html#aa74b1b34478ef0fde22cd029366ab3faa1c0cd5d0cf074aff5336fcd076047c66", null ],
      [ "HF_PWM_MODE_FADE", "EspTypes__PWM_8h.html#aa74b1b34478ef0fde22cd029366ab3faafcaebf50ed45950d02944973268665f3", null ]
    ] ],
    [ "hf_pwm_resolution_t", "EspTypes__PWM_8h.html#a12cb9c68b937b77fee8dca891ac3978d", [
      [ "HF_PWM_RES_8BIT", "EspTypes__PWM_8h.html#a12cb9c68b937b77fee8dca891ac3978dad276ad183f79937aa3271496335f3b14", null ],
      [ "HF_PWM_RES_10BIT", "EspTypes__PWM_8h.html#a12cb9c68b937b77fee8dca891ac3978dad84c8db92a605ec5fae4422635fe030e", null ],
      [ "HF_PWM_RES_12BIT", "EspTypes__PWM_8h.html#a12cb9c68b937b77fee8dca891ac3978da1fde7afa684ff8d33e642a0bc039dd3e", null ],
      [ "HF_PWM_RES_14BIT", "EspTypes__PWM_8h.html#a12cb9c68b937b77fee8dca891ac3978da8d3b8ccf67ec159821cd5374a07222ee", null ]
    ] ],
    [ "HF_PWM_APB_CLOCK_HZ", "EspTypes__PWM_8h.html#afc05cebb7e350df0e040f5ee863e733b", null ],
    [ "HF_PWM_DEFAULT_FREQUENCY", "EspTypes__PWM_8h.html#ae522afd7305f23439344ed06a817091c", null ],
    [ "HF_PWM_DEFAULT_RESOLUTION", "EspTypes__PWM_8h.html#a92cb7b8188635483e620a8cf14826a04", null ],
    [ "HF_PWM_MAX_CHANNELS", "EspTypes__PWM_8h.html#a86d24d14ec124f4012f812f9853993f0", null ],
    [ "HF_PWM_MAX_FREQUENCY", "EspTypes__PWM_8h.html#aefdd9851ea138af7ce3b87a861c9f8b7", null ],
    [ "HF_PWM_MAX_RESOLUTION", "EspTypes__PWM_8h.html#ae635150b452b279b29b65f799961d51f", null ],
    [ "HF_PWM_MAX_TIMERS", "EspTypes__PWM_8h.html#a188b45a8dc354e4753a2d2a3ade6b06e", null ],
    [ "HF_PWM_MIN_FREQUENCY", "EspTypes__PWM_8h.html#ab59b928c49e8f4ef0dbc7c1bac390c93", null ]
];