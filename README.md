# ! INDEV ! Do not use in production

## HTTP Server Logger

A logger that forwards the logs to a web dashboard.

### Basic Integration

```cpp
#include "Logger.hpp"

int main() {
    Server server(5000); // input port
    Logger logger(server, 1, "Logger"); // Server(server_instance), int(logger_id), std::string(logger_name)
    server.Start();

    // Now log using macros
    LOG_INFO(logger, "Info Log");
    LOG_WARN(logger, "Warning Log");
    LOG_ERROR(logger, "Error Log");
    LOG_CUSTOM(logger, "Custom Message", "CUSTOM1");

    return 0;
}
```
