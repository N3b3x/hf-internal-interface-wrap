var BaseGpio_8h =
[
    [ "HF_GPIO_ERR_LIST", "group__gpio.html#gacdef61b325fdcc83f9974bc7c3c6d800", null ],
    [ "X", "BaseGpio_8h.html#a94b225babb1626cee0621f8312eb32d1", null ],
    [ "X", "BaseGpio_8h.html#a94b225babb1626cee0621f8312eb32d1", null ],
    [ "InterruptCallback", "group__gpio.html#ga1327f68d8287dd473a8c6cf0333a989f", null ],
    [ "hf_gpio_active_state_t", "group__gpio.html#gaf02cdaf150fa829e4a871e58ed772c6d", [
      [ "HF_GPIO_ACTIVE_LOW", "group__gpio.html#ggaf02cdaf150fa829e4a871e58ed772c6da450278e3db6a65478bdb5a456f824c42", null ],
      [ "HF_GPIO_ACTIVE_HIGH", "group__gpio.html#ggaf02cdaf150fa829e4a871e58ed772c6dab2e7e26dbb35ac48971828008f86b356", null ]
    ] ],
    [ "hf_gpio_direction_t", "group__gpio.html#ga6b3450e1c9d6cba3355cc3567bb0cce3", [
      [ "HF_GPIO_DIRECTION_INPUT", "group__gpio.html#gga6b3450e1c9d6cba3355cc3567bb0cce3a5090403a0f4f177d6fb813cd9d06bf1e", null ],
      [ "HF_GPIO_DIRECTION_OUTPUT", "group__gpio.html#gga6b3450e1c9d6cba3355cc3567bb0cce3ae8b58d24a05839b6d1c0305c945748b0", null ]
    ] ],
    [ "hf_gpio_err_t", "group__gpio.html#ga2632aac2351807c35e790ec20bda305d", [
      [ "X", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da4edf38c36b44ec6289313be3dbff3bd1", null ],
      [ "GPIO_SUCCESS", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da08c6dcbce56949318695e40344555937", null ],
      [ "GPIO_ERR_FAILURE", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da3e74dd36bf6d066da5a6cd611f23277b", null ],
      [ "GPIO_ERR_NOT_INITIALIZED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305dab04683ad857b2517e5793490e3568bce", null ],
      [ "GPIO_ERR_ALREADY_INITIALIZED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305daf0a93e17339a18d4f9483db8be996892", null ],
      [ "GPIO_ERR_INVALID_PARAMETER", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da91fd58f5ca1f07c7ac4582826b9efdd3", null ],
      [ "GPIO_ERR_NULL_POINTER", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da256954b890b4465ff3e01a68a1742812", null ],
      [ "GPIO_ERR_OUT_OF_MEMORY", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da425ad913151330d4ad51535e7e31d30a", null ],
      [ "GPIO_ERR_INVALID_PIN", "group__gpio.html#gga2632aac2351807c35e790ec20bda305dae129022a9db54a23a0f2b5b9c678d40a", null ],
      [ "GPIO_ERR_PIN_NOT_FOUND", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da833caab2ac64c22894b0e5415d3e4168", null ],
      [ "GPIO_ERR_PIN_NOT_CONFIGURED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da665cd6866bb8bf54cb5c1eb42f68ba20", null ],
      [ "GPIO_ERR_PIN_ALREADY_REGISTERED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305dab988dbee738e95b8c802b4fc6754bbd6", null ],
      [ "GPIO_ERR_PIN_ACCESS_DENIED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da963337aa12ed713ecee24d8bc0bc1af7", null ],
      [ "GPIO_ERR_PIN_BUSY", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da583f31f55f292d9460d4401997878135", null ],
      [ "GPIO_ERR_HARDWARE_FAULT", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da9d05c78e2d6dbd4868a3c7ad13ed5c25", null ],
      [ "GPIO_ERR_COMMUNICATION_FAILURE", "group__gpio.html#gga2632aac2351807c35e790ec20bda305dac830e46415a4ec395ce2b9a9f1a2fd62", null ],
      [ "GPIO_ERR_DEVICE_NOT_RESPONDING", "group__gpio.html#gga2632aac2351807c35e790ec20bda305dae656a8d5b2e8d547c0d976cc883cb601", null ],
      [ "GPIO_ERR_TIMEOUT", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da60701f049c086b44b9b07455b31b32d4", null ],
      [ "GPIO_ERR_VOLTAGE_OUT_OF_RANGE", "group__gpio.html#gga2632aac2351807c35e790ec20bda305dad08bc7a57b45033bc186de679f17d3fa", null ],
      [ "GPIO_ERR_INVALID_CONFIGURATION", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da56d5db816b34f92a07c27dc2b22035be", null ],
      [ "GPIO_ERR_UNSUPPORTED_OPERATION", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da8cc09234750055376887ad35240174e5", null ],
      [ "GPIO_ERR_RESOURCE_BUSY", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da3b9274154bbc74e912aee32a8be38573", null ],
      [ "GPIO_ERR_RESOURCE_UNAVAILABLE", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da3d8c84f6917cd95b346b4c9269466274", null ],
      [ "GPIO_ERR_READ_FAILURE", "group__gpio.html#gga2632aac2351807c35e790ec20bda305dad373c8c390069c2855b78d797b2b114a", null ],
      [ "GPIO_ERR_WRITE_FAILURE", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da875f2148cc6ff4a4e5c5ac14ef468218", null ],
      [ "GPIO_ERR_DIRECTION_MISMATCH", "group__gpio.html#gga2632aac2351807c35e790ec20bda305daf325d6fbe617f4a9a26c320d3d3f26b0", null ],
      [ "GPIO_ERR_PULL_RESISTOR_FAILURE", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da7784f100f9881c8950cb9376118c533c", null ],
      [ "GPIO_ERR_INTERRUPT_NOT_SUPPORTED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da21fcfb07c03413755abfbcee7fd7f267", null ],
      [ "GPIO_ERR_INTERRUPT_ALREADY_ENABLED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da081cd9145412fbcccd5e59c5dfff8515", null ],
      [ "GPIO_ERR_INTERRUPT_NOT_ENABLED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305daded9e1e9c5cba44f5dd775b76c2c7eb8", null ],
      [ "GPIO_ERR_INTERRUPT_HANDLER_FAILED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da48c59a175bbf7e75d7a633ab8c9f5ee0", null ],
      [ "GPIO_ERR_SYSTEM_ERROR", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da1924675e5ce4a6f35862c3170b2cf82c", null ],
      [ "GPIO_ERR_PERMISSION_DENIED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da64dd337c77c8ad344b81077d1563a75f", null ],
      [ "GPIO_ERR_OPERATION_ABORTED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da36813840681f05663478922b67509143", null ],
      [ "GPIO_ERR_NOT_SUPPORTED", "group__gpio.html#gga2632aac2351807c35e790ec20bda305dad0dedbb315ad871e12c7da9e293a37dd", null ],
      [ "GPIO_ERR_DRIVER_ERROR", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da335a7094845a2813f8b06105d3f3e210", null ],
      [ "GPIO_ERR_INVALID_STATE", "group__gpio.html#gga2632aac2351807c35e790ec20bda305daedd8db4c5482eb9b06a93f7686e7df5a", null ],
      [ "GPIO_ERR_INVALID_ARG", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da88adf359e7e9360c9b7a03ca100ce46b", null ],
      [ "GPIO_ERR_CALIBRATION_FAILURE", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da90cb9acd4ee0be850b46e00382ebda3e", null ],
      [ "GPIO_ERR_UNKNOWN", "group__gpio.html#gga2632aac2351807c35e790ec20bda305dae95d515b07b7e1ff6706de3ebc08bd50", null ],
      [ "GPIO_ERR_COUNT", "group__gpio.html#gga2632aac2351807c35e790ec20bda305da2736a20c86d944fdf231c360a49b98d8", null ]
    ] ],
    [ "hf_gpio_interrupt_trigger_t", "group__gpio.html#ga7830c017a6fb46b8478c7ca44940c3c1", [
      [ "HF_GPIO_INTERRUPT_TRIGGER_NONE", "group__gpio.html#gga7830c017a6fb46b8478c7ca44940c3c1a599debe50453c8a67ce2ce7dfb455fa6", null ],
      [ "HF_GPIO_INTERRUPT_TRIGGER_RISING_EDGE", "group__gpio.html#gga7830c017a6fb46b8478c7ca44940c3c1a3329c6966749e885e0fae014078c306b", null ],
      [ "HF_GPIO_INTERRUPT_TRIGGER_FALLING_EDGE", "group__gpio.html#gga7830c017a6fb46b8478c7ca44940c3c1a5aed939da4e6fc63381ce0a51ce58e9d", null ],
      [ "HF_GPIO_INTERRUPT_TRIGGER_BOTH_EDGES", "group__gpio.html#gga7830c017a6fb46b8478c7ca44940c3c1a09801f149bee7cb99caad5d5e649abc3", null ],
      [ "HF_GPIO_INTERRUPT_TRIGGER_LOW_LEVEL", "group__gpio.html#gga7830c017a6fb46b8478c7ca44940c3c1adea2885dba61754e32fed0ce5e6657c6", null ],
      [ "HF_GPIO_INTERRUPT_TRIGGER_HIGH_LEVEL", "group__gpio.html#gga7830c017a6fb46b8478c7ca44940c3c1a5e5303d5ffc82721bd4158217156dde0", null ]
    ] ],
    [ "hf_gpio_level_t", "group__gpio.html#ga04d416163750773ac08d092bd0d4038e", [
      [ "HF_GPIO_LEVEL_LOW", "group__gpio.html#gga04d416163750773ac08d092bd0d4038ea0e6f86bd5369fde81e8064b7865ea9e3", null ],
      [ "HF_GPIO_LEVEL_HIGH", "group__gpio.html#gga04d416163750773ac08d092bd0d4038ea38ff7115dcf0a8897f602d7a63861f17", null ]
    ] ],
    [ "hf_gpio_output_mode_t", "group__gpio.html#ga825412a54660defc9ecbf8ad1ea1cf7b", [
      [ "HF_GPIO_OUTPUT_MODE_PUSH_PULL", "group__gpio.html#gga825412a54660defc9ecbf8ad1ea1cf7ba26b362f67afa6dbc264fb539c0dc304d", null ],
      [ "HF_GPIO_OUTPUT_MODE_OPEN_DRAIN", "group__gpio.html#gga825412a54660defc9ecbf8ad1ea1cf7ba5f14b6a9f5f28544b4314b7dfd2cdf51", null ]
    ] ],
    [ "hf_gpio_pull_mode_t", "group__gpio.html#ga7d27555a7050f5d9d9006c96b841e335", [
      [ "HF_GPIO_PULL_MODE_FLOATING", "group__gpio.html#gga7d27555a7050f5d9d9006c96b841e335ae17c8cfbde2e4323ba15750747758875", null ],
      [ "HF_GPIO_PULL_MODE_UP", "group__gpio.html#gga7d27555a7050f5d9d9006c96b841e335a88a10405bfdfad3cb5c0f3ee531a54eb", null ],
      [ "HF_GPIO_PULL_MODE_DOWN", "group__gpio.html#gga7d27555a7050f5d9d9006c96b841e335addecfb46009ce8c5ad563e8fbab0a749", null ],
      [ "HF_GPIO_PULL_MODE_UP_DOWN", "group__gpio.html#gga7d27555a7050f5d9d9006c96b841e335ad8fb8979e9fdbec7cc4e842f23abcdb1", null ]
    ] ],
    [ "hf_gpio_state_t", "group__gpio.html#ga49490004a4935c1f8f727fcbfba7f887", [
      [ "HF_GPIO_STATE_INACTIVE", "group__gpio.html#gga49490004a4935c1f8f727fcbfba7f887a4646f94b6d7499976b7c997e826b95dc", null ],
      [ "HF_GPIO_STATE_ACTIVE", "group__gpio.html#gga49490004a4935c1f8f727fcbfba7f887a14da1ccdb89e74c932383b4d4cd86573", null ]
    ] ],
    [ "HfGpioErrToString", "group__gpio.html#gafc39fe16e57bc415f05e1f8bf7c4e0be", null ]
];