#ifndef RMT_OUTPUT_H
#define RMT_OUTPUT_H

#include "driver/rmt.h"
#include "driver/gpio.h"

/**
 * @file RmtOutput.h
 * @brief Simple wrapper around the ESP-IDF RMT API for TX channels.
 */
class RmtOutput {
public:
    RmtOutput(rmt_channel_t channel, gpio_num_t pin, uint32_t clk_div = 80) noexcept;
    ~RmtOutput() noexcept;
    RmtOutput(const RmtOutput&) = delete;
    RmtOutput& operator=(const RmtOutput&) = delete;

    bool Open() noexcept;
    void Close() noexcept;

    bool Write(const rmt_item32_t* items, size_t len, bool wait_tx_done = true) noexcept;

    bool IsOpen() const noexcept { return installed; }

private:
    rmt_channel_t chan;
    gpio_num_t gpio;
    uint32_t div;
    bool installed;
};

#endif // RMT_OUTPUT_H
