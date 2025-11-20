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
