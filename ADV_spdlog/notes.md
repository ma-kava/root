# Learning to use `spdlog` library

### how to build `main.cpp`
```c++
g++ main.cpp -o main -lspdlog -lfmt
```
### dulezite insights
> spdlog's sinks have _mt (multi threaded) or _st (single threaded) suffixes to indicate the thread safety.

```c++
logger->info(...)  
    ↓  
logger->log(...)  
    ↓  
sink->log(msg)          // volá SPDLOG
    ↓  
sink->sink_it_(msg)     // vaše implementace
    ↓  
formatter_->format(...) // vyrobí string
    ↓  
client_.send(...)       // pošle UDP
```

