/**
 * @file UsbSerialJtagComprehensiveTest.cpp
 * @brief Comprehensive test suite for the EspUsbSerialJtag wrapper (noexcept).
 *
 * Targets ESP32-S3 / -C3 / -C6 / -H2 / -P4 — any chip with a built-in
 * USB Serial/JTAG controller. The test exercises the full BaseUsbSerialJtag
 * surface:
 *
 *   - construction / lazy lifecycle
 *   - share-an-IDF-console-driver coexistence path
 *   - basic Write / Read paths (with optional host-side echo loop)
 *   - WriteString / WriteLine / ReadByte convenience helpers
 *   - FlushTx semantics
 *   - IsHostConnected diagnostics
 *   - statistics counters
 *   - error-handling guard rails (null pointer, zero length, not-init)
 *   - clean teardown vs. shared-driver teardown
 *
 * Plug a host PC into the chip's native USB port (the same one used for
 * `idf.py monitor`). The interactive sub-tests will look for an echo from
 * the host and time out cleanly when running unattended in CI.
 *
 * Build:
 *   ./scripts/build_app.sh usb_serial_jtag_test Release
 *
 * @author HardFOC
 * @date 2026
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "base/BaseUsbSerialJtag.h"
#include "mcu/esp32/EspUsbSerialJtag.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#ifdef __cplusplus
}
#endif

#include <cstdio>
#include <cstring>

static const char* TAG = "USJ_Test";
static TestResults g_test_results;

//=============================================================================
// TEST SECTION CONFIGURATION
//=============================================================================

static constexpr bool ENABLE_CORE_TESTS       = true;  // construction, init, share path
static constexpr bool ENABLE_IO_TESTS         = true;  // Write / Read / helpers / flush
static constexpr bool ENABLE_DIAG_TESTS       = true;  // host-connected, statistics
static constexpr bool ENABLE_ERROR_TESTS      = true;  // null/zero/not-init guard rails
static constexpr bool ENABLE_INTERACTIVE_TESTS = true; // optional host-side echo loop
static constexpr bool ENABLE_CLEANUP_TESTS    = true;  // deinit semantics

// How long to wait for the host to echo back during the optional echo test.
static constexpr uint32_t HOST_ECHO_TIMEOUT_MS = 2'000;

// Forward declarations
bool test_usj_construction() noexcept;
bool test_usj_lazy_initialize() noexcept;
bool test_usj_idf_share_path() noexcept;
bool test_usj_write_basic() noexcept;
bool test_usj_write_string_and_line() noexcept;
bool test_usj_read_timeout_when_idle() noexcept;
bool test_usj_flush_tx() noexcept;
bool test_usj_host_connected_query() noexcept;
bool test_usj_statistics_increment() noexcept;
bool test_usj_null_pointer_guards() noexcept;
bool test_usj_zero_length_is_success() noexcept;
bool test_usj_not_initialized_returns_error() noexcept;
bool test_usj_interactive_echo() noexcept;
bool test_usj_safe_deinitialize() noexcept;

//=============================================================================
// HELPERS
//=============================================================================

namespace {

/// Drain any leftover RX bytes so subsequent reads start from a known state.
void drain_rx(BaseUsbSerialJtag& s, uint32_t max_ms = 50) noexcept {
    uint8_t scratch[64];
    uint32_t got = 0;
    (void)s.Read(scratch, sizeof(scratch), max_ms, &got);
}

/// Acquire (or re-acquire) the singleton + ensure it's initialised.
/// Returns nullptr only if the platform driver refuses to install — at
/// which point every downstream test should bail.
BaseUsbSerialJtag* acquire() noexcept {
    auto& s = EspUsbSerialJtag::Default();
    if (!s.EnsureInitialized()) {
        return nullptr;
    }
    return &s;
}

}  // namespace

//=============================================================================
// CORE LIFECYCLE
//=============================================================================

bool test_usj_construction() noexcept {
    // Construction must NOT install the driver (matches BaseUart contract).
    EspUsbSerialJtag local{};
    if (local.IsInitialized()) {
        ESP_LOGE(TAG, "freshly constructed wrapper reports IsInitialized()=true");
        return false;
    }
    return true;
}

bool test_usj_lazy_initialize() noexcept {
    auto& s = EspUsbSerialJtag::Default();
    if (!s.EnsureInitialized()) {
        ESP_LOGE(TAG, "EnsureInitialized() failed");
        return false;
    }
    if (!s.IsInitialized()) {
        ESP_LOGE(TAG, "IsInitialized() should be true after EnsureInitialized()");
        return false;
    }
    // Idempotent
    if (!s.EnsureInitialized()) {
        ESP_LOGE(TAG, "second EnsureInitialized() failed");
        return false;
    }
    return true;
}

bool test_usj_idf_share_path() noexcept {
    // If CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y was set in sdkconfig, the IDF
    // console installed the driver before app_main() ran. The wrapper must
    // detect that and share, never failing with ESP_ERR_INVALID_STATE.
    const bool already = usb_serial_jtag_is_driver_installed();
    ESP_LOGI(TAG, "usb_serial_jtag_is_driver_installed() = %s",
             already ? "true (IDF console owns it)" : "false (we own it)");

    auto* s = acquire();
    if (!s) {
        ESP_LOGE(TAG, "acquire() returned nullptr");
        return false;
    }
    // Either ownership outcome is valid; we only assert the wrapper accepted
    // the install and the underlying driver is now live.
    if (!usb_serial_jtag_is_driver_installed()) {
        ESP_LOGE(TAG, "driver still not installed after EnsureInitialized()");
        return false;
    }
    return true;
}

//=============================================================================
// I/O
//=============================================================================

bool test_usj_write_basic() noexcept {
    auto* s = acquire();
    if (!s) return false;

    static constexpr uint8_t payload[] = "USJ:basic-write\r\n";
    const auto e = s->Write(payload, sizeof(payload) - 1);
    if (e != hf_uart_err_t::UART_SUCCESS) {
        ESP_LOGE(TAG, "Write(basic) returned %d", static_cast<int>(e));
        return false;
    }
    return true;
}

bool test_usj_write_string_and_line() noexcept {
    auto* s = acquire();
    if (!s) return false;

    if (s->WriteString("USJ:WriteString-no-newline ") != hf_uart_err_t::UART_SUCCESS) {
        return false;
    }
    if (s->WriteLine("USJ:WriteLine-with-CRLF") != hf_uart_err_t::UART_SUCCESS) {
        return false;
    }
    return true;
}

bool test_usj_read_timeout_when_idle() noexcept {
    auto* s = acquire();
    if (!s) return false;

    drain_rx(*s);

    uint8_t b{};
    uint32_t got = 0;
    const auto e = s->Read(&b, 1, /*timeout_ms=*/50, &got);
    // If the host happens to be typing, accept SUCCESS; otherwise we expect
    // TIMEOUT. Anything else is a real failure.
    if (e == hf_uart_err_t::UART_SUCCESS) {
        ESP_LOGI(TAG, "Read returned a stray byte (got=%lu, byte=0x%02X) — accepting",
                 static_cast<unsigned long>(got), b);
        return true;
    }
    if (e != hf_uart_err_t::UART_ERR_TIMEOUT) {
        ESP_LOGE(TAG, "expected UART_ERR_TIMEOUT on idle read, got %d", static_cast<int>(e));
        return false;
    }
    if (got != 0) {
        ESP_LOGE(TAG, "TIMEOUT reported but out_read_count=%lu", static_cast<unsigned long>(got));
        return false;
    }
    return true;
}

bool test_usj_flush_tx() noexcept {
    auto* s = acquire();
    if (!s) return false;

    // Push a small burst then ask the driver to drain.
    for (int i = 0; i < 8; ++i) {
        char line[32];
        const int n = std::snprintf(line, sizeof(line), "USJ:flush-burst %d\r\n", i);
        if (s->Write(reinterpret_cast<const hf_u8_t*>(line),
                     static_cast<hf_u32_t>(n)) != hf_uart_err_t::UART_SUCCESS) {
            return false;
        }
    }

    const auto e = s->FlushTx(/*timeout_ms=*/200);
    // If no host is enumerated the ring may not actually drain — both
    // SUCCESS and TIMEOUT are acceptable here. Anything else is a bug.
    if (e != hf_uart_err_t::UART_SUCCESS && e != hf_uart_err_t::UART_ERR_TIMEOUT) {
        ESP_LOGE(TAG, "FlushTx returned unexpected code %d", static_cast<int>(e));
        return false;
    }
    return true;
}

//=============================================================================
// DIAGNOSTICS
//=============================================================================

bool test_usj_host_connected_query() noexcept {
    auto* s = acquire();
    if (!s) return false;

    // Two reads should be consistent within the same polling window. We don't
    // assert true/false (depends on the operator); we only check stability.
    const bool a = s->IsHostConnected();
    const bool b = s->IsHostConnected();
    ESP_LOGI(TAG, "IsHostConnected: %s / %s", a ? "true" : "false", b ? "true" : "false");
    if (a != b) {
        ESP_LOGW(TAG, "host-connected flag flapped between back-to-back calls");
        // Don't fail — USB enumeration genuinely can race. Just note it.
    }
    return true;
}

bool test_usj_statistics_increment() noexcept {
    auto* s = acquire();
    if (!s) return false;

    const auto before = s->GetStatistics();

    static constexpr uint8_t blob[] = "USJ:stats-bump\r\n";
    if (s->Write(blob, sizeof(blob) - 1) != hf_uart_err_t::UART_SUCCESS) {
        return false;
    }

    const auto after = s->GetStatistics();
    if (after.tx_byte_count <= before.tx_byte_count) {
        ESP_LOGE(TAG, "tx_byte_count did not increment (before=%llu, after=%llu)",
                 static_cast<unsigned long long>(before.tx_byte_count),
                 static_cast<unsigned long long>(after.tx_byte_count));
        return false;
    }
    return true;
}

//=============================================================================
// ERROR HANDLING / GUARDS
//=============================================================================

bool test_usj_null_pointer_guards() noexcept {
    auto* s = acquire();
    if (!s) return false;

    if (s->Write(nullptr, 4) != hf_uart_err_t::UART_ERR_NULL_POINTER) return false;
    if (s->Read(nullptr, 4)  != hf_uart_err_t::UART_ERR_NULL_POINTER) return false;
    if (s->WriteString(nullptr) != hf_uart_err_t::UART_ERR_NULL_POINTER) return false;
    return true;
}

bool test_usj_zero_length_is_success() noexcept {
    auto* s = acquire();
    if (!s) return false;

    uint8_t dummy = 0xA5;
    if (s->Write(&dummy, 0) != hf_uart_err_t::UART_SUCCESS) return false;
    if (s->Read(&dummy, 0)  != hf_uart_err_t::UART_SUCCESS) return false;
    return true;
}

bool test_usj_not_initialized_returns_error() noexcept {
    // Construct a fresh, unrelated instance; do NOT initialise it.
    EspUsbSerialJtag fresh{};
    uint8_t b = 0;
    if (fresh.Write(&b, 1) != hf_uart_err_t::UART_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "Write on uninitialised wrapper should return NOT_INITIALIZED");
        return false;
    }
    if (fresh.Read(&b, 1)  != hf_uart_err_t::UART_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "Read on uninitialised wrapper should return NOT_INITIALIZED");
        return false;
    }
    if (fresh.FlushTx(0) != hf_uart_err_t::UART_ERR_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "FlushTx on uninitialised wrapper should return NOT_INITIALIZED");
        return false;
    }
    if (fresh.IsHostConnected()) {
        ESP_LOGE(TAG, "IsHostConnected on uninitialised wrapper should be false");
        return false;
    }
    return true;
}

//=============================================================================
// INTERACTIVE
//=============================================================================

bool test_usj_interactive_echo() noexcept {
    auto* s = acquire();
    if (!s) return false;

    if (!s->IsHostConnected()) {
        ESP_LOGW(TAG, "no host enumerated — skipping interactive echo (this is OK in CI)");
        return true;
    }

    drain_rx(*s);

    s->WriteLine("USJ:echo-test (type any character within 2s)");

    uint8_t b{};
    const auto e = s->ReadByte(b, HOST_ECHO_TIMEOUT_MS);
    if (e == hf_uart_err_t::UART_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "no host input within %lu ms — skipping (still PASS)",
                 static_cast<unsigned long>(HOST_ECHO_TIMEOUT_MS));
        return true;
    }
    if (e != hf_uart_err_t::UART_SUCCESS) {
        ESP_LOGE(TAG, "ReadByte returned %d", static_cast<int>(e));
        return false;
    }

    char line[48];
    const int n = std::snprintf(line, sizeof(line), "USJ:got 0x%02X ('%c')\r\n",
                                b, (b >= 0x20 && b < 0x7F) ? static_cast<char>(b) : '?');
    return s->Write(reinterpret_cast<const hf_u8_t*>(line),
                    static_cast<hf_u32_t>(n)) == hf_uart_err_t::UART_SUCCESS;
}

//=============================================================================
// CLEANUP
//=============================================================================

bool test_usj_safe_deinitialize() noexcept {
    auto& s = EspUsbSerialJtag::Default();

    // Whatever we did above, EnsureDeinitialized must succeed AND must not
    // tear down a driver we didn't install (e.g. the IDF console). The
    // wrapper's owns_driver_ flag is what prevents that.
    if (!s.EnsureDeinitialized()) {
        ESP_LOGE(TAG, "EnsureDeinitialized() failed");
        return false;
    }
    if (s.IsInitialized()) {
        ESP_LOGE(TAG, "IsInitialized() should be false after EnsureDeinitialized()");
        return false;
    }

    // Re-arm the singleton so the post-test banner can still print over USB.
    if (!s.EnsureInitialized()) {
        ESP_LOGW(TAG, "could not re-initialise after teardown — final banner may not appear");
    }
    return true;
}

//=============================================================================
// app_main
//=============================================================================

extern "C" void app_main(void) {
    ESP_LOGI(TAG,
             "╔════════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG,
             "║                ESP32 USB SERIAL/JTAG COMPREHENSIVE TEST SUITE                  ║");
    ESP_LOGI(TAG,
             "║                         HardFOC Internal Interface                             ║");
    ESP_LOGI(TAG,
             "╚════════════════════════════════════════════════════════════════════════════════╝");

    vTaskDelay(pdMS_TO_TICKS(500));

    print_test_section_status(TAG, "USB_SERIAL_JTAG");

    RUN_TEST_SECTION_IF_ENABLED(
        ENABLE_CORE_TESTS, "USJ CORE TESTS",
        ESP_LOGI(TAG, "Running construction / lifecycle tests...");
        RUN_TEST_IN_TASK("construction",       test_usj_construction,    8192, 1);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("lazy_initialize",    test_usj_lazy_initialize, 8192, 1);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("idf_share_path",     test_usj_idf_share_path,  8192, 1);
        flip_test_progress_indicator();
    );

    RUN_TEST_SECTION_IF_ENABLED(
        ENABLE_IO_TESTS, "USJ I/O TESTS",
        ESP_LOGI(TAG, "Running Write / Read / FlushTx tests...");
        RUN_TEST_IN_TASK("write_basic",          test_usj_write_basic,            8192, 1);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("write_string_and_line",test_usj_write_string_and_line,  8192, 1);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("read_timeout_when_idle",test_usj_read_timeout_when_idle,8192, 1);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("flush_tx",             test_usj_flush_tx,               8192, 1);
        flip_test_progress_indicator();
    );

    RUN_TEST_SECTION_IF_ENABLED(
        ENABLE_DIAG_TESTS, "USJ DIAGNOSTICS TESTS",
        ESP_LOGI(TAG, "Running host-connected / statistics tests...");
        RUN_TEST_IN_TASK("host_connected_query", test_usj_host_connected_query, 8192, 1);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("statistics_increment", test_usj_statistics_increment, 8192, 1);
        flip_test_progress_indicator();
    );

    RUN_TEST_SECTION_IF_ENABLED(
        ENABLE_ERROR_TESTS, "USJ ERROR HANDLING TESTS",
        ESP_LOGI(TAG, "Running guard-rail tests...");
        RUN_TEST_IN_TASK("null_pointer_guards",       test_usj_null_pointer_guards,        8192, 1);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("zero_length_is_success",    test_usj_zero_length_is_success,     8192, 1);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("not_initialized_returns_error",
                         test_usj_not_initialized_returns_error, 8192, 1);
        flip_test_progress_indicator();
    );

    RUN_TEST_SECTION_IF_ENABLED(
        ENABLE_INTERACTIVE_TESTS, "USJ INTERACTIVE TESTS",
        ESP_LOGI(TAG, "Running interactive echo (skips automatically when no host present)...");
        RUN_TEST_IN_TASK("interactive_echo", test_usj_interactive_echo, 8192, 1);
        flip_test_progress_indicator();
    );

    RUN_TEST_SECTION_IF_ENABLED(
        ENABLE_CLEANUP_TESTS, "USJ CLEANUP TESTS",
        ESP_LOGI(TAG, "Running cleanup tests...");
        RUN_TEST_IN_TASK("safe_deinitialize", test_usj_safe_deinitialize, 8192, 1);
        flip_test_progress_indicator();
    );

    print_test_summary(g_test_results, "USB_SERIAL_JTAG", TAG);

    ESP_LOGI(TAG, "USB Serial/JTAG comprehensive testing completed.");
    ESP_LOGI(TAG, "System will continue running. Press RESET to restart tests.");

    while (true) {
        ESP_LOGI(TAG, "System up and running for %lld seconds",
                 esp_timer_get_time() / 1000000LL);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
