#include "RmtOutput.h"

RmtOutput::RmtOutput(rmt_channel_t channel, gpio_num_t pin, uint32_t clk_div) noexcept
    : chan(channel), gpio(pin), div(clk_div), installed(false) {}

RmtOutput::~RmtOutput() noexcept {
    Close();
}

bool RmtOutput::Open() noexcept {
    if (installed) return true;
    rmt_config_t cfg = {};
    cfg.rmt_mode = RMT_MODE_TX;
    cfg.channel = chan;
    cfg.gpio_num = gpio;
    cfg.clk_div = div;
    cfg.mem_block_num = 1;
    if (rmt_config(&cfg) != ESP_OK) {
        return false;
    }
    if (rmt_driver_install(chan, 0, 0) != ESP_OK) {
        return false;
    }
    installed = true;
    return true;
}

void RmtOutput::Close() noexcept {
    if (installed) {
        rmt_driver_uninstall(chan);
        installed = false;
    }
}

bool RmtOutput::Write(const rmt_item32_t* items, size_t len, bool wait_tx_done) noexcept {
    if (!installed) return false;
    return rmt_write_items(chan, items, len, wait_tx_done) == ESP_OK;
}

