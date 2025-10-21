# ! INDEV ! Do not use in production

## HTTP Server Logger

A lightweight HTTP server-based logging library for C++ that serves as a replacement for traditional `print` or `cout` statements. This project provides real-time log viewing through a web interface.

## TODO

- Python library port
- Python wrapper for C++ library
- Additional logging features and customization options

## Overview

HTTP Server Logger is designed to provide developers with a modern, web-based approach to application logging. Instead of cluttering your terminal or log files, logs are served through an HTTP server and can be viewed in any web browser.

## Project Structure

```sh
http-python-logger/
├── src/                    # Source files
│   ├── Logger.cpp         # Logger implementation
│   ├── Logger.hpp         # Logger header
│   ├── Server.cpp         # HTTP server implementation
│   ├── Server.hpp         # HTTP server header
│   ├── main.cpp           # Main entry point
│   ├── embedded/          # Embedded resources
│   └── utils/             # Utility functions
├── external/              # External dependencies
│   ├── included/          # Included libraries
│   └── vcpkg/            # vcpkg package manager
├── build/                 # Build output directory
├── .vscode/              # VS Code configuration
├── CMakeLists.txt        # CMake build configuration
├── CMakePresets.json     # CMake presets
├── setup.sh              # Setup script
├── run.sh                # Run script
├── embed_build.sh        # Embedded resource build script
└── README.md             # This file
```

## Features

- **HTTP-based logging**: View logs in real-time through a web browser
- **Easy integration**: Simple API to replace traditional logging methods

## Building the Project

### Prerequisites

- CMake 4.0 or higher
- C++23 compatible compiler (GCC, Clang, or MSVC)
- vcpkg (included as submodule)

### Setup

1. Clone the repository:

    ```bash
    git clone https://github.com/Vesprr/http-python-logger
    cd http-python-logger
    ```

2. Run the setup script:

    ```bash
    ./setup.sh
    ```

## Usage

### Running the Server

```bash
./run.sh
```

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

## Development

### CMake Presets

The project uses CMake presets defined in CMakePresets.json for consistent build configurations.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

---

For more information about the project structure and implementation details, refer to the source files in the src directory.
