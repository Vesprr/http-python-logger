### USAGE:
- In your C++ file add: `#include "embeds.hpp"`
- Use `[yourfilename]_str` to access the file as string; returns `const char *` inlined
- Use `[yourfilename]_len` to access the length of the file in bytes; returns `const unsigned int` inlined

### ADDING_YOUR_FILE:

- Move the file to embed into `files` directory
- Run `embed_build.sh` at root
- Open embeds.hpp
  - Do `#include "yourfilename.extension.inc"`
  - In Embedded namespace, create two variables
    - `inline const char *[yourfilename]_str = reinterpret_cast<const char *>([yourfilename]);`
    - `inline const unsigned int [yourfilename]_len = [yourfilename]_len;`

### TEMPLATE:

```cpp
#pragma once
#include "[yourfilename].[extension].inc"
.
.
.

namespace Embedded
{
    inline const char *[yourfilename]_str = reinterpret_cast<const char *>([yourfilename]);
    inline const unsigned int [yourfilename]_len = [yourfilename]_len;
    .
    .
    .
}
```

### WORKING:

- Loops through all files in `files` directory
- Check if files ends with suffix that needs to be excluded; current ones are "~", ".bak", ".tmp", ".md"
- If file is to be excluded move on to next file otherwise continue on with the loop
- Use `xxd` to convert the file to C array holding binary data
  - `xxd` stores the array and its length in `incs/[yourfilename].[extension].inc`
- Exit the loop
- Output times
