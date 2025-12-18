# HTTP sink message local transfer

First run `python server.py`, then compile and run `main.cpp`.

### Compilation
```c++
g++ main.cpp -o sender -lcurl -lspdlog -lfmt
```

### **Scheme**
```
┌────────────────────────────┐
│          main.cpp          │
│     (produces and sends    │
│       error messages)      │
└─────────────┬──────────────┘
              | HTTP
              ▼
┌──────────────────────────────┐
│    localhost python server   │
│        prints messages       │
└──────────────────────────────┘
```

# HTTP zip file transfer
## How to send a folder with logs in it
**Approach:**
- Take all that is to send, that is, find the `logs` folder
- Create a ZIP from it
- Send it via HTTP POST to the Flask server (`server.py`)
