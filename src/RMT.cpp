#include "RMT.hpp"

RMT::RMT(rmt_channel_t channel, gpio_num_t pin, uint32_t clk_div) noexcept
    : chan(channel), gpio(pin), div(clk_div) {}

RMT::~RMT() noexcept {
  Close();
}

bool RMT::OpenTx() noexcept {
  if (tx)
    return true;
  uint32_t resolution = 80'000'000 / div;
  tx = std::make_unique<iid::RmtTx>(gpio, resolution, 64, false, 4, RMT_CLK_SRC_DEFAULT,
                                    static_cast<int>(chan));
  return true;
}

bool RMT::OpenRx(uint32_t idle_threshold_us, uint32_t filter_ns) noexcept {
  if (rx)
    return true;
  uint32_t resolution = 80'000'000 / div;
  rx = std::make_unique<iid::RmtRx>(gpio, resolution, 64, idle_threshold_us, filter_ns,
                                    RMT_CLK_SRC_DEFAULT, static_cast<int>(chan));
  return true;
}

void RMT::Close() noexcept {
  tx.reset();
  rx.reset();
}

bool RMT::Write(const rmt_symbol_word_t *items, size_t len, bool wait_tx_done) noexcept {
  (void)wait_tx_done;
  if (!tx)
    return false;
  return tx->transmit(items, len) == ESP_OK;
}

bool RMT::Receive(rmt_symbol_word_t *buffer, size_t buffer_symbols, size_t *out_symbols) noexcept {
  if (!rx)
    return false;
  return rx->receive(buffer, buffer_symbols, out_symbols) == ESP_OK;
}
