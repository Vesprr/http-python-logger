#pragma once
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

namespace utils {

inline void initTerminal() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO); // disable buffering and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

inline void resetTerminal() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO); // restore default settings
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

inline bool keyPressed(char &ch) {
    int oldf = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK); // non-blocking

    int c = getchar();
    if (c != EOF) {
        ch = static_cast<char>(c);
        fcntl(STDIN_FILENO, F_SETFL, oldf); // restore flags
        return true;
    }

    fcntl(STDIN_FILENO, F_SETFL, oldf); // restore flags
    return false;
}

} // namespace utils
