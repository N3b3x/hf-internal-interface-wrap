/**
 * @file StmTypes.h
 * @ingroup stm32_types
 * @brief STM32 platform-specific type definitions for hardware abstraction.
 *
 * This header provides the bridge between STM32 HAL (CubeMX-generated) types and
 * the HardFOC base class interfaces. Users integrate their CubeMX projects by
 * passing STM32 HAL handles (e.g., `ADC_HandleTypeDef*`, `I2C_HandleTypeDef*`)
 * into the Stm* wrapper constructors.
 *
 * @section Design Design Philosophy
 * STM32 CubeMX generates peripheral initialization code that populates HAL handle
 * structs. The HardFOC wrappers accept these handles — they do NOT re-initialize
 * the hardware. This provides:
 * - Zero-friction integration with existing CubeMX projects
 * - Full compatibility with any STM32 family (F0/F1/F3/F4/F7/G0/G4/H5/H7/L0/L4/U5/WB/WL)
 * - Users retain full control of clock trees, pin-muxing, and peripheral config
 *
 * @section Usage Usage
 * @code
 * // In your CubeMX-generated main.c (or after MX_xxx_Init calls):
 * extern ADC_HandleTypeDef hadc1;
 * extern I2C_HandleTypeDef hi2c1;
 * extern SPI_HandleTypeDef hspi1;
 *
 * StmAdc adc(&hadc1);
 * StmI2cBus i2cBus(&hi2c1);
 * StmSpiBus spiBus(&hspi1);
 * @endcode
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "HardwareTypes.h"
#include <cstdint>
#include <cstddef>

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════
// These forward-declare STM32 HAL handle types so that StmTypes.h can be
// included WITHOUT pulling in the full stm32xxxx_hal.h.  The actual definitions
// are provided by the user's CubeMX-generated HAL includes.
// ─────────────────────────────────────────────────────────────────────────────

/// @cond INTERNAL
struct ADC_HandleTypeDef;
struct I2C_HandleTypeDef;
struct SPI_HandleTypeDef;
struct TIM_HandleTypeDef;
struct UART_HandleTypeDef;
struct CAN_HandleTypeDef;       // CAN1/CAN2 (STM32F1/F2/F4)
struct FDCAN_HandleTypeDef;     // FDCAN (STM32G4/H7/U5)
struct DAC_HandleTypeDef;
struct DMA_HandleTypeDef;
struct GPIO_TypeDef;            // GPIOA, GPIOB, etc.

// Flash/NVS-related — STM32 doesn't have a handle, just addresses
// We wrap flash sector operations internally.
/// @endcond

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 PLATFORM CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════════

namespace hf::stm32 {

/// @brief Maximum GPIO ports (A..K = 11 on largest STM32H7)
static constexpr hf_u8_t kMaxGpioPorts = 11;

/// @brief Maximum pins per GPIO port
static constexpr hf_u8_t kPinsPerPort = 16;

/// @brief Maximum total GPIO pins (ports * pins_per_port)
static constexpr hf_u16_t kMaxGpioPins = kMaxGpioPorts * kPinsPerPort;

/// @brief ADC maximum channels (typical STM32F4/H7)
static constexpr hf_u8_t kAdcMaxChannels = 20;

/// @brief ADC maximum resolution (bits)
static constexpr hf_u8_t kAdcMaxResolutionBits = 16;  // STM32H7
static constexpr hf_u8_t kAdcDefaultResolutionBits = 12;

/// @brief ADC reference voltage (V) — most STM32 boards
static constexpr float kAdcDefaultVrefV = 3.3f;

/// @brief I2C limits
static constexpr hf_u32_t kI2cMaxFreqHz = 1000000;    // Fast-mode Plus
static constexpr hf_u32_t kI2cStandardFreqHz = 100000;
static constexpr hf_u32_t kI2cFastFreqHz = 400000;
static constexpr hf_u32_t kI2cFastPlusFreqHz = 1000000;
static constexpr hf_u16_t kI2cMaxTransferBytes = 65535;

/// @brief SPI limits
static constexpr hf_u32_t kSpiMaxFreqHz = 50000000;   // Depends on family
static constexpr hf_u32_t kSpiDefaultFreqHz = 1000000;
static constexpr hf_u16_t kSpiMaxTransferBytes = 65535;

/// @brief UART limits
static constexpr hf_u32_t kUartMaxBaudRate = 10000000;  // STM32H7 LPUART
static constexpr hf_u32_t kUartDefaultBaudRate = 115200;
static constexpr hf_u16_t kUartDefaultBufferSize = 256;

/// @brief Timer limits (in microseconds)
static constexpr hf_u64_t kTimerMinPeriodUs = 1;
static constexpr hf_u64_t kTimerMaxPeriodUs = 4294967295ULL; // 32-bit counter @ 1 MHz
static constexpr hf_u64_t kTimerResolutionUs = 1;

/// @brief PWM limits
static constexpr hf_u32_t kPwmMaxFreqHz = 1000000;
static constexpr hf_u32_t kPwmMinFreqHz = 1;
static constexpr hf_u16_t kPwmMaxResolution = 65535; // 16-bit timer
static constexpr hf_u8_t kPwmMaxChannelsPerTimer = 4;

/// @brief CAN limits
static constexpr hf_u32_t kCanMaxBaudRate = 1000000;
static constexpr hf_u32_t kCanFdMaxBaudRate = 8000000;
static constexpr hf_u16_t kCanDefaultQueueSize = 16;

/// @brief Flash/NVS limits
static constexpr size_t kNvsMaxKeyLength = 64;
static constexpr size_t kNvsMaxValueSize = 4096;
static constexpr size_t kNvsDefaultSectorSize = 4096; // Typical flash sector

/// @brief Temperature sensor
static constexpr float kTempSensorMinC = -40.0f;
static constexpr float kTempSensorMaxC = 125.0f;
static constexpr float kTempSensorTypAccuracyC = 1.5f;

/// @brief Logger
static constexpr hf_u32_t kLoggerMaxMessageLen = 256;
static constexpr hf_u32_t kLoggerDefaultBufferSize = 1024;

}  // namespace hf::stm32

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 GPIO PORT/PIN ENCODING
// ═══════════════════════════════════════════════════════════════════════════════
// STM32 GPIO is addressed as (Port, Pin) — e.g., PA5 = Port A, Pin 5.
// We encode this into hf_pin_num_t as: pin_num = (port_index * 16) + pin_index
// where port_index: A=0, B=1, C=2, ..., K=10.
// This allows a single hf_pin_num_t to uniquely identify any STM32 GPIO pin.
// ─────────────────────────────────────────────────────────────────────────────

namespace hf::stm32 {

/// @brief GPIO port index (A=0, B=1, ... K=10)
enum class GpioPort : hf_u8_t {
    A = 0, B = 1, C = 2, D = 3, E = 4,
    F = 5, G = 6, H = 7, I = 8, J = 9, K = 10
};

/// @brief Encode (port, pin) → hf_pin_num_t
constexpr hf_pin_num_t EncodePin(GpioPort port, hf_u8_t pin) noexcept {
    return static_cast<hf_pin_num_t>(static_cast<hf_u8_t>(port) * kPinsPerPort + pin);
}

/// @brief Decode hf_pin_num_t → port index
constexpr hf_u8_t DecodePinPort(hf_pin_num_t pin) noexcept {
    return static_cast<hf_u8_t>(pin / kPinsPerPort);
}

/// @brief Decode hf_pin_num_t → pin within port (0-15)
constexpr hf_u8_t DecodePinIndex(hf_pin_num_t pin) noexcept {
    return static_cast<hf_u8_t>(pin % kPinsPerPort);
}

/// @brief Decode hf_pin_num_t → STM32 HAL pin mask (GPIO_PIN_0 .. GPIO_PIN_15)
constexpr hf_u16_t DecodePinMask(hf_pin_num_t pin) noexcept {
    return static_cast<hf_u16_t>(1U << DecodePinIndex(pin));
}

/// @brief Convenience aliases for common pins (PA0, PA1, ... PK15)
namespace pin {
    constexpr hf_pin_num_t PA0  = EncodePin(GpioPort::A, 0);
    constexpr hf_pin_num_t PA1  = EncodePin(GpioPort::A, 1);
    constexpr hf_pin_num_t PA2  = EncodePin(GpioPort::A, 2);
    constexpr hf_pin_num_t PA3  = EncodePin(GpioPort::A, 3);
    constexpr hf_pin_num_t PA4  = EncodePin(GpioPort::A, 4);
    constexpr hf_pin_num_t PA5  = EncodePin(GpioPort::A, 5);
    constexpr hf_pin_num_t PA6  = EncodePin(GpioPort::A, 6);
    constexpr hf_pin_num_t PA7  = EncodePin(GpioPort::A, 7);
    constexpr hf_pin_num_t PA8  = EncodePin(GpioPort::A, 8);
    constexpr hf_pin_num_t PA9  = EncodePin(GpioPort::A, 9);
    constexpr hf_pin_num_t PA10 = EncodePin(GpioPort::A, 10);
    constexpr hf_pin_num_t PA11 = EncodePin(GpioPort::A, 11);
    constexpr hf_pin_num_t PA12 = EncodePin(GpioPort::A, 12);
    constexpr hf_pin_num_t PA13 = EncodePin(GpioPort::A, 13);
    constexpr hf_pin_num_t PA14 = EncodePin(GpioPort::A, 14);
    constexpr hf_pin_num_t PA15 = EncodePin(GpioPort::A, 15);

    constexpr hf_pin_num_t PB0  = EncodePin(GpioPort::B, 0);
    constexpr hf_pin_num_t PB1  = EncodePin(GpioPort::B, 1);
    constexpr hf_pin_num_t PB2  = EncodePin(GpioPort::B, 2);
    constexpr hf_pin_num_t PB3  = EncodePin(GpioPort::B, 3);
    constexpr hf_pin_num_t PB4  = EncodePin(GpioPort::B, 4);
    constexpr hf_pin_num_t PB5  = EncodePin(GpioPort::B, 5);
    constexpr hf_pin_num_t PB6  = EncodePin(GpioPort::B, 6);
    constexpr hf_pin_num_t PB7  = EncodePin(GpioPort::B, 7);
    constexpr hf_pin_num_t PB8  = EncodePin(GpioPort::B, 8);
    constexpr hf_pin_num_t PB9  = EncodePin(GpioPort::B, 9);
    constexpr hf_pin_num_t PB10 = EncodePin(GpioPort::B, 10);
    constexpr hf_pin_num_t PB11 = EncodePin(GpioPort::B, 11);
    constexpr hf_pin_num_t PB12 = EncodePin(GpioPort::B, 12);
    constexpr hf_pin_num_t PB13 = EncodePin(GpioPort::B, 13);
    constexpr hf_pin_num_t PB14 = EncodePin(GpioPort::B, 14);
    constexpr hf_pin_num_t PB15 = EncodePin(GpioPort::B, 15);

    constexpr hf_pin_num_t PC0  = EncodePin(GpioPort::C, 0);
    constexpr hf_pin_num_t PC1  = EncodePin(GpioPort::C, 1);
    constexpr hf_pin_num_t PC2  = EncodePin(GpioPort::C, 2);
    constexpr hf_pin_num_t PC3  = EncodePin(GpioPort::C, 3);
    constexpr hf_pin_num_t PC4  = EncodePin(GpioPort::C, 4);
    constexpr hf_pin_num_t PC5  = EncodePin(GpioPort::C, 5);
    constexpr hf_pin_num_t PC6  = EncodePin(GpioPort::C, 6);
    constexpr hf_pin_num_t PC7  = EncodePin(GpioPort::C, 7);
    constexpr hf_pin_num_t PC8  = EncodePin(GpioPort::C, 8);
    constexpr hf_pin_num_t PC9  = EncodePin(GpioPort::C, 9);
    constexpr hf_pin_num_t PC10 = EncodePin(GpioPort::C, 10);
    constexpr hf_pin_num_t PC11 = EncodePin(GpioPort::C, 11);
    constexpr hf_pin_num_t PC12 = EncodePin(GpioPort::C, 12);
    constexpr hf_pin_num_t PC13 = EncodePin(GpioPort::C, 13);
    constexpr hf_pin_num_t PC14 = EncodePin(GpioPort::C, 14);
    constexpr hf_pin_num_t PC15 = EncodePin(GpioPort::C, 15);

    constexpr hf_pin_num_t PD0  = EncodePin(GpioPort::D, 0);
    constexpr hf_pin_num_t PD1  = EncodePin(GpioPort::D, 1);
    constexpr hf_pin_num_t PD2  = EncodePin(GpioPort::D, 2);
}  // namespace pin

}  // namespace hf::stm32

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 I2C CONFIGURATION TYPES
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief I2C address mode
enum class hf_stm32_i2c_addr_mode_t : hf_u8_t {
    ADDR_7BIT  = 0,  ///< 7-bit addressing
    ADDR_10BIT = 1   ///< 10-bit addressing
};

/// @brief Platform-agnostic I2C bus configuration for STM32.
/// Users pass their CubeMX-generated I2C_HandleTypeDef* here.
struct hf_i2c_bus_config_t {
    I2C_HandleTypeDef* hal_handle;    ///< Pointer to CubeMX-generated I2C handle
    hf_u32_t default_timeout_ms;      ///< Default timeout for all operations (ms)
    bool use_dma;                     ///< Use DMA transfers when available

    hf_i2c_bus_config_t() noexcept
        : hal_handle(nullptr), default_timeout_ms(1000), use_dma(false) {}

    explicit hf_i2c_bus_config_t(I2C_HandleTypeDef* handle,
                                  hf_u32_t timeout_ms = 1000,
                                  bool dma = false) noexcept
        : hal_handle(handle), default_timeout_ms(timeout_ms), use_dma(dma) {}
};

/// @brief I2C device configuration for STM32.
struct hf_i2c_device_config_t {
    hf_u16_t device_address;                             ///< 7-bit or 10-bit address
    hf_stm32_i2c_addr_mode_t addr_mode;                  ///< Address mode
    hf_u32_t scl_speed_hz;                               ///< SCL clock speed (informational)
    bool disable_ack_check;                               ///< Disable ACK checking

    hf_i2c_device_config_t() noexcept
        : device_address(0),
          addr_mode(hf_stm32_i2c_addr_mode_t::ADDR_7BIT),
          scl_speed_hz(100000),
          disable_ack_check(false) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 SPI CONFIGURATION TYPES
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief SPI mode (CPOL/CPHA)
enum class hf_stm32_spi_mode_t : hf_u8_t {
    MODE_0 = 0,  ///< CPOL=0, CPHA=0
    MODE_1 = 1,  ///< CPOL=0, CPHA=1
    MODE_2 = 2,  ///< CPOL=1, CPHA=0
    MODE_3 = 3   ///< CPOL=1, CPHA=1
};

/// @brief Platform-agnostic SPI bus configuration for STM32.
struct hf_spi_bus_config_t {
    SPI_HandleTypeDef* hal_handle;     ///< Pointer to CubeMX-generated SPI handle
    hf_u32_t default_timeout_ms;       ///< Default timeout (ms)
    bool use_dma;                      ///< Use DMA transfers when available

    hf_spi_bus_config_t() noexcept
        : hal_handle(nullptr), default_timeout_ms(1000), use_dma(false) {}

    explicit hf_spi_bus_config_t(SPI_HandleTypeDef* handle,
                                  hf_u32_t timeout_ms = 1000,
                                  bool dma = false) noexcept
        : hal_handle(handle), default_timeout_ms(timeout_ms), use_dma(dma) {}
};

/// @brief SPI device configuration for STM32.
struct hf_spi_device_config_t {
    hf_u32_t clock_speed_hz;          ///< Clock speed (informational — set in CubeMX)
    hf_stm32_spi_mode_t mode;         ///< SPI mode (informational — set in CubeMX)
    GPIO_TypeDef* cs_port;             ///< CS GPIO port (e.g., GPIOA)
    hf_u16_t cs_pin;                   ///< CS GPIO pin mask (e.g., GPIO_PIN_4)
    bool cs_active_low;                ///< CS active low (default: true)

    hf_spi_device_config_t() noexcept
        : clock_speed_hz(1000000), mode(hf_stm32_spi_mode_t::MODE_0),
          cs_port(nullptr), cs_pin(0), cs_active_low(true) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 UART CONFIGURATION TYPES
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief UART data bits
enum class hf_stm32_uart_data_bits_t : hf_u8_t {
    DATA_7_BITS = 7,
    DATA_8_BITS = 8,
    DATA_9_BITS = 9
};

/// @brief UART parity
enum class hf_stm32_uart_parity_t : hf_u8_t {
    NONE = 0,
    EVEN = 1,
    ODD  = 2
};

/// @brief UART stop bits
enum class hf_stm32_uart_stop_bits_t : hf_u8_t {
    STOP_1   = 0,
    STOP_2   = 1
};

/// @brief Platform-agnostic UART configuration for STM32.
struct hf_stm32_uart_config_t {
    UART_HandleTypeDef* hal_handle;       ///< Pointer to CubeMX-generated UART handle
    hf_u32_t default_timeout_ms;          ///< Default timeout (ms)
    bool use_dma_tx;                      ///< Use DMA for TX
    bool use_dma_rx;                      ///< Use DMA for RX
    hf_u16_t rx_buffer_size;              ///< Receive ring buffer size
    hf_u16_t tx_buffer_size;              ///< Transmit buffer size

    hf_stm32_uart_config_t() noexcept
        : hal_handle(nullptr), default_timeout_ms(1000),
          use_dma_tx(false), use_dma_rx(false),
          rx_buffer_size(hf::stm32::kUartDefaultBufferSize),
          tx_buffer_size(hf::stm32::kUartDefaultBufferSize) {}

    explicit hf_stm32_uart_config_t(UART_HandleTypeDef* handle,
                                     hf_u32_t timeout_ms = 1000) noexcept
        : hal_handle(handle), default_timeout_ms(timeout_ms),
          use_dma_tx(false), use_dma_rx(false),
          rx_buffer_size(hf::stm32::kUartDefaultBufferSize),
          tx_buffer_size(hf::stm32::kUartDefaultBufferSize) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 CAN CONFIGURATION TYPES
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief CAN bus type selection
enum class hf_stm32_can_type_t : hf_u8_t {
    BXCAN  = 0,  ///< Basic Extended CAN (STM32F1/F2/F4)
    FDCAN  = 1   ///< Flexible Data-rate CAN (STM32G4/H7/U5)
};

/// @brief CAN configuration for STM32.
struct hf_stm32_can_config_t {
    hf_stm32_can_type_t can_type;      ///< CAN peripheral type
    void* hal_handle;                   ///< CAN_HandleTypeDef* or FDCAN_HandleTypeDef*
    hf_u32_t default_timeout_ms;        ///< Default timeout (ms)
    bool loopback_mode;                 ///< Enable loopback for testing
    bool silent_mode;                   ///< Enable silent/listen-only mode

    hf_stm32_can_config_t() noexcept
        : can_type(hf_stm32_can_type_t::BXCAN), hal_handle(nullptr),
          default_timeout_ms(1000), loopback_mode(false), silent_mode(false) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 PWM CONFIGURATION TYPES
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief PWM channel identifier (maps to TIM_CHANNEL_1..4)
enum class hf_stm32_pwm_channel_t : hf_u32_t {
    CHANNEL_1 = 0x00000000U,   ///< TIM_CHANNEL_1
    CHANNEL_2 = 0x00000004U,   ///< TIM_CHANNEL_2
    CHANNEL_3 = 0x00000008U,   ///< TIM_CHANNEL_3
    CHANNEL_4 = 0x0000000CU    ///< TIM_CHANNEL_4
};

/// @brief PWM configuration for STM32.
struct hf_stm32_pwm_config_t {
    TIM_HandleTypeDef* hal_handle;   ///< Pointer to CubeMX-generated TIM handle
    hf_u8_t num_channels;            ///< Number of active channels (1-4)
    bool complementary_output;       ///< Enable complementary outputs (TIMx_CHxN)
    hf_u16_t dead_time_ns;           ///< Dead-time for complementary pairs (ns)

    hf_stm32_pwm_config_t() noexcept
        : hal_handle(nullptr), num_channels(1),
          complementary_output(false), dead_time_ns(0) {}

    explicit hf_stm32_pwm_config_t(TIM_HandleTypeDef* handle,
                                    hf_u8_t channels = 1) noexcept
        : hal_handle(handle), num_channels(channels),
          complementary_output(false), dead_time_ns(0) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 ADC CONFIGURATION TYPES
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief ADC resolution
enum class hf_stm32_adc_resolution_t : hf_u8_t {
    BITS_6  = 6,
    BITS_8  = 8,
    BITS_10 = 10,
    BITS_12 = 12,
    BITS_14 = 14,   ///< STM32H7/G4 only
    BITS_16 = 16    ///< STM32H7 only
};

/// @brief ADC configuration for STM32.
struct hf_stm32_adc_config_t {
    ADC_HandleTypeDef* hal_handle;             ///< Pointer to CubeMX-generated ADC handle
    hf_stm32_adc_resolution_t resolution;      ///< ADC resolution
    float vref_v;                              ///< Reference voltage (V)
    hf_u8_t num_channels;                      ///< Number of configured channels

    hf_stm32_adc_config_t() noexcept
        : hal_handle(nullptr),
          resolution(hf_stm32_adc_resolution_t::BITS_12),
          vref_v(hf::stm32::kAdcDefaultVrefV),
          num_channels(1) {}

    explicit hf_stm32_adc_config_t(ADC_HandleTypeDef* handle,
                                    hf_u8_t channels = 1,
                                    float vref = hf::stm32::kAdcDefaultVrefV) noexcept
        : hal_handle(handle),
          resolution(hf_stm32_adc_resolution_t::BITS_12),
          vref_v(vref),
          num_channels(channels) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 NVS (FLASH) CONFIGURATION TYPES
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief Flash-based NVS configuration for STM32.
struct hf_stm32_nvs_config_t {
    hf_u32_t flash_start_address;    ///< Start address of NVS flash region
    hf_u32_t flash_size;             ///< Total NVS region size (bytes)
    hf_u32_t sector_size;            ///< Flash sector/page size (bytes)
    hf_u8_t num_sectors;             ///< Number of sectors reserved for NVS

    hf_stm32_nvs_config_t() noexcept
        : flash_start_address(0), flash_size(0),
          sector_size(hf::stm32::kNvsDefaultSectorSize), num_sectors(2) {}

    hf_stm32_nvs_config_t(hf_u32_t start_addr, hf_u32_t size,
                           hf_u32_t sect_size = hf::stm32::kNvsDefaultSectorSize) noexcept
        : flash_start_address(start_addr), flash_size(size),
          sector_size(sect_size),
          num_sectors(static_cast<hf_u8_t>(size / sect_size)) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 TIMER CONFIGURATION TYPES
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief Periodic timer configuration for STM32.
struct hf_stm32_timer_config_t {
    TIM_HandleTypeDef* hal_handle;   ///< Pointer to CubeMX-generated TIM handle
    hf_u32_t prescaler;              ///< Timer prescaler (0 = auto-compute)
    bool use_interrupt;              ///< Use timer update interrupt

    hf_stm32_timer_config_t() noexcept
        : hal_handle(nullptr), prescaler(0), use_interrupt(true) {}

    explicit hf_stm32_timer_config_t(TIM_HandleTypeDef* handle) noexcept
        : hal_handle(handle), prescaler(0), use_interrupt(true) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 GPIO CONFIGURATION TYPE
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief GPIO configuration for STM32 wrapping CubeMX port/pin.
struct hf_stm32_gpio_config_t {
    GPIO_TypeDef* port;              ///< GPIO port pointer (GPIOA, GPIOB, etc.)
    hf_u16_t pin_mask;               ///< GPIO pin mask (GPIO_PIN_0..GPIO_PIN_15)

    hf_stm32_gpio_config_t() noexcept : port(nullptr), pin_mask(0) {}

    hf_stm32_gpio_config_t(GPIO_TypeDef* gpio_port, hf_u16_t pin) noexcept
        : port(gpio_port), pin_mask(pin) {}
};

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL STATUS CONVERSION UTILITY
// ═══════════════════════════════════════════════════════════════════════════════

namespace hf::stm32 {

/// @brief STM32 HAL status codes (mirrors HAL_StatusTypeDef without HAL include)
enum class HalStatus : hf_u8_t {
    OK      = 0x00U,
    ERROR   = 0x01U,
    BUSY    = 0x02U,
    TIMEOUT = 0x03U
};

/// @brief Convert raw HAL_StatusTypeDef to our enum
constexpr HalStatus ToHalStatus(hf_u32_t raw_status) noexcept {
    switch (raw_status) {
        case 0: return HalStatus::OK;
        case 1: return HalStatus::ERROR;
        case 2: return HalStatus::BUSY;
        case 3: return HalStatus::TIMEOUT;
        default: return HalStatus::ERROR;
    }
}

/// @brief Check if a raw HAL status indicates success
constexpr bool IsHalOk(hf_u32_t raw_status) noexcept {
    return raw_status == 0;
}

}  // namespace hf::stm32
