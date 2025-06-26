# 🛡️ Error Handling Guide

Robust applications require consistent error reporting. The HAL uses a set of `hf_err_t` codes returned by most functions.

## Checking Results

```cpp
hf_err_t err = gpio.Open();
if (err != HF_OK) {
    // handle failure
}
```

All errors are negative values while `HF_OK` equals zero.

## Common Error Codes

| Code | Meaning |
|-----:|---------|
| `HF_ERR_INVALID_ARG` | Invalid parameter was passed |
| `HF_ERR_TIMEOUT` | Operation timed out |
| `HF_ERR_NOT_SUPPORTED` | Feature is unavailable on current hardware |
| `HF_ERR_NO_MEM` | Memory allocation failed |

Refer to `include/base/HfError.h` for the complete list.

## Best Practices

- Propagate error codes upward rather than swallowing them.
- Use logging to provide additional context.
- When possible, recover from transient errors such as timeouts.

## 🧰 Advanced Techniques
- **Scoped Checks**: Wrap calls in helper macros to log and return on failure
- **Assertions**: Use `HF_ASSERT` for conditions that should never fail
- **Retries**: Implement retry loops for transient communication errors

## 🔗 Related Examples
- [📝 Data Logger](../examples/data-logger.md)
- [🏭 Industrial I/O](../examples/industrial-io.md)
