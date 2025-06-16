#include "RmtOutput.h"

RmtOutput::RmtOutput(rmt_channel_t channel, gpio_num_t pin, uint32_t clk_div) noexcept
    : chan(channel), gpio(pin), div(clk_div), installed(false) {}

RmtOutput::~RmtOutput() noexcept {
  Close();
}

bool RmtOutput::Open() noexcept {
  if (installed)
    return true;
  uint32_t resolution = 80'000'000 / div;
  tx = std::make_unique<iid::RmtTx>(gpio, resolution, 64, false, 4, RMT_CLK_SRC_DEFAULT,
                                    static_cast<int>(chan));
  installed = true;
  return true;
}

void RmtOutput::Close() noexcept {
  tx.reset();
  installed = false;
}

bool RmtOutput::Write(const rmt_item32_t *items, size_t len, bool wait_tx_done) noexcept {
  (void)wait_tx_done;
  if (!installed || !tx)
    return false;
  return tx->transmit(reinterpret_cast<const rmt_symbol_word_t *>(items), len) == ESP_OK;
}
