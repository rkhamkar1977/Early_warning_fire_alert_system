/*
 * web.h
 *
 *  Created on: 2016/10/7
 *      Author: User
 */

#ifndef WEB_H_
#define WEB_H_

#include "web_src/_web.h"

const char *html_head =
        "HTTP/1.1 200 OK\n\
Accept-Ranges: bytes\n\
Content-Length: %d\n\
Connection: close\n\
Content-Type: text/html\n\
\n\
";

const char *favicon_ico_head =
        "HTTP/1.1 200 OK\n\
Accept-Ranges: bytes\n\
Content-Length: %d\n\
Connection: close\n\
Content-Type: text/plain\n\
\n\
";
const char *jpg_head =
        "HTTP/1.1 200 OK\n\
Accept-Ranges: bytes\n\
Content-Length: %d\n\
Connection: close\n\
Content-Type: image/jpeg\n\
\n\
";
const char *__302_head =
        "HTTP/1.1 302 Found\n\
Location: /\n\
Accept-Ranges: bytes\n\
Content-Length: %d\n\
Connection: close\n\
Content-Type: text/html\n\
\n\
";

unsigned char __302_html[] = "\r\n";
unsigned int __302_html_len = 2;

const char *__404_head =
        "HTTP/1.1 Not Found\n\
Accept-Ranges: bytes\n\
Content-Length: %d\n\
Connection: close\n\
Content-Type: text/html\n\
\n\
";
#endif /* WEB_H_ */
