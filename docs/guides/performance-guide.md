# ⚡ Performance Guide

Optimizing for real-time motor control requires careful tuning. This guide lists strategies to minimize latency and CPU load.

## General Tips

- Prefer static allocation over dynamic memory to avoid fragmentation.
- Use the MCU's DMA capabilities for SPI and UART transfers.
- Minimize logging in time-critical sections.

## GPIO and Interrupts

- Keep ISR handlers short and defer work to tasks.
- Use `DigitalOutputGuard` to toggle pins without overhead.

## ADC Sampling

- Configure the ADC sampling frequency appropriately; oversampling wastes cycles.
- Batch multiple channels using `ReadSequence()` when available.

## Thread Safety vs Speed

- Thread-safe wrappers add minimal overhead but consider using them only when needed.
- Lock only around short critical sections.

For detailed porting considerations see the [Porting Guide](porting-guide.md).

## ⚙️ Memory Layout
- Keep frequently accessed data in IRAM when possible
- Use `static` or `constexpr` to place constants in flash

## 🚀 Zero-Copy Techniques
- DMA-friendly APIs avoid copying buffers
- For SPI and UART, allocate aligned memory for best throughput

## 🔗 Related Examples
- [🏎️ Motor Control](../examples/motor-control.md)
