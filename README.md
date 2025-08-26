# cpp-template
C++ template with `vcpkg` as `git submodule` and setup with `CMake`.

- Project was created on MacOS, has not been tested on Windows or Linux
- Uses `vcpkg`
- Run `./setup` to setup the project i.e installing packages
- use `./run` for running afterwards


send message in the following format: 

first two bytes: logger id 
next 8 bytes: level
next 2 bytes: line_number
next 2 bytes: length of func 
next 2 bytes: length of file_name 
next N bytes: for time use a predefined fixed size here 
next func bytes 
next file_name bytes

