# HwMiniPix CPU Command – Test Suite

This project provides **unit tests** and **integration tests** for the `CpuCmds` library.  
It validates correct **serialization**, **parsing**, **execution logic**, and **error handling** of all CPU commands.

---

## Project Goals

- Ensure **protocol correctness** (commands match MCU spec).
- Validate **resilience** against malformed or truncated responses.
- Provide **mock-based unit tests** for fast CI.
- Support **integration tests** against either:
  - a real MiniPIX device, or
  - a simulator (fake MCU).

---

## Architecture of Testing
```pgsql
 ┌─────────────────────────────┐
 │   Application logic         │  ← wants "Get temperature", "Enable power"
 └───────────┬─────────────────┘
             │
 ┌───────────▼─────────────────┐
 │ CpuCmds (your file)         │  ← knows how to serialize/parse commands
 │   - serialize(cmd)          │
 │   - execute(transport, cmd) │
 └───────────┬─────────────────┘
             │
 ┌───────────▼─────────────────┐
 │ ICpuTransport               │  ← abstraction of a “wire” to the device
 │   - send()                  │
 │   - recv()                  │
 │   - exchange()              │
 └───────────┬─────────────────┘
             │
 ┌───────────▼─────────────────┐
 │ Real transport (Serial, USB)│  ← actually talks to MCU
 └─────────────────────────────┘
```
---

## Types of Tests

### Unit Tests
- **Serialization tests**  
  - Each `serialize(cmd)` produces the exact expected byte buffer.
- **Execution tests with mocks**  
  - `PostedBehavior` → verifies `send()` called correctly.  
  - `AckedBehavior` → verifies `exchange()` returns ACK/NACK correctly.  
  - `RespondedBehavior` → simulates valid & broken responses.  
  - `VarLen` → simulates multi-step protocol (length + data).  
- **Resilience tests**  
  - Broken magic headers.  
  - Truncated or oversized responses.  
  - Unterminated strings.  
  - Random garbage payloads.

### Integration Tests
- Run only when hardware/simulator available.  
- Use real `SerialTransport` → `/dev/ttyUSBx` or Windows COM port.  
- Smoke-test subset of commands:
  - `ReadCpuFirmwareVersion`  
  - `GetCpuTemperature`  
  - `ChipPowerEnable`  
- Tagged in CTest so they can be skipped in CI:
```bash
  ctest -L integration
```