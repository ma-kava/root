# HwMiniPix CPU Command – Test Suite

This project provides **unit tests** and **integration tests** for the `hwlibs/minipix/tpx2/cpucommands.h` library.  
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
 │ cpucommands.h               │  ← knows how to serialize/parse commands
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

### **How to Test Each Aspect**

#### 1. **Test Serialization (Unit Test, No Mock)**
- Just call `serialize(cmd)` and check the output matches expected bytes.
#### 2. **Test Communication (App → MCU, with Mock)**
- Mock `ICpuTransport::exchange()`.
- Ensure your `execute()` function serializes correctly and calls `exchange()` with the right send buffer.
- Optionally, simulate a response buffer in the mock to feed into parsing.
#### 3. **Test Parsing (Unit Test, No Mock)**
- Call `parse(cmd, recv_buf)` with a known buffer and check parsed struct/fields.
#### 4. **Integration-like Test (End-to-End Roundtrip, with Fake)**
- For true end-to-end, use a **fake** (not a mock) ICpuTransport that mimics MCU behavior:
    - On `exchange`, it receives command bytes, checks if they're correct, and returns a canned response.
    - Your test then checks the parsed result.
### **Summary Table**

| Test          | What it Checks                           | Mock/Real? |
| ------------- | ---------------------------------------- | ---------- |
| Serialization | serialize() output                       | None       |
| Communication | App calls transport with correct buffers | Mock       |
| Parsing       | parse() works on known buffer            | None       |
| End-to-end    | Everything together                      | Fake       |
