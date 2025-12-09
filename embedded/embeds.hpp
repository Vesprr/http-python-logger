#pragma once
#include "server.html.inc"

namespace Embedded
{
    inline const char *server_html_str = reinterpret_cast<const char *>(server_html);
    inline const unsigned int server_html_len = server_html_len;
}