/* Copyright 2009,2010 Ryan Dahl <ry@tinyclouds.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef http_parser_h
#define http_parser_h
#ifdef __cplusplus
extern "C" {
#endif


#include <sys/types.h>


/* Compile with -DHTTP_PARSER_STRICT=0 to make less checks, but run
 * faster
 */
#ifndef HTTP_PARSER_STRICT
# define HTTP_PARSER_STRICT 1
#else
# define HTTP_PARSER_STRICT 0
#endif


/* Maximium header size allowed */
#define HTTP_MAX_HEADER_SIZE (80*1024)


typedef struct http_parser http_parser;
typedef struct http_parser_settings http_parser_settings;


/* Callbacks should return non-zero to indicate an error. The parser will
 * then halt execution.
 *
 * The one exception is on_headers_complete. In a HTTP_RESPONSE parser
 * returning '1' from on_headers_complete will tell the parser that it
 * should not expect a body. This is used when receiving a response to a
 * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
 * chunked' headers that indicate the presence of a body.
 *
 * http_data_cb does not return data chunks. It will be call arbitrarally
 * many times for each string. E.G. you might get 10 callbacks for "on_path"
 * each providing just a few characters more data.
 */
typedef int (*http_data_cb) (http_parser*, const char *at, size_t length);
typedef int (*http_cb) (http_parser*);


/* Should be at least one longer than the longest request method */
#define HTTP_PARSER_MAX_METHOD_LEN 10


/* Request Methods */
enum http_method
  { HTTP_DELETE    = 0x0001
  , HTTP_GET       = 0x0002
  , HTTP_HEAD      = 0x0004
  , HTTP_POST      = 0x0008
  , HTTP_PUT       = 0x0010
  /* pathological */
  , HTTP_CONNECT   = 0x0020
  , HTTP_OPTIONS   = 0x0040
  , HTTP_TRACE     = 0x0080
  /* webdav */
  , HTTP_COPY      = 0x0100
  , HTTP_LOCK      = 0x0200
  , HTTP_MKCOL     = 0x0400
  , HTTP_MOVE      = 0x0800
  , HTTP_PROPFIND  = 0x1000
  , HTTP_PROPPATCH = 0x2000
  , HTTP_UNLOCK    = 0x4000
  };


enum http_parser_type { HTTP_REQUEST, HTTP_RESPONSE, HTTP_BOTH };


struct http_parser {
  /** PRIVATE **/
  unsigned char type;
  unsigned char state;
  unsigned char header_state;
  unsigned char index;

  char flags;

  size_t nread;
  ssize_t body_read;
  ssize_t content_length;

  size_t      current_mark_size;
  const char *header_field_mark;
  const char *header_value_mark;
  const char *query_string_mark;
  const char *path_mark;
  const char *url_mark;
  const char *fragment_mark;

  /** READ-ONLY **/
  unsigned short status_code; /* responses only */
  unsigned short method;    /* requests only */
  unsigned short http_major;
  unsigned short http_minor;
  char buffer[HTTP_PARSER_MAX_METHOD_LEN];

  /* 1 = Upgrade header was present and the parser has exited because of that.
   * 0 = No upgrade header present.
   * Should be checked when http_parser_execute() returns in addition to
   * error checking.
   */
  char upgrade;

  /** PUBLIC **/
  void *data; /* A pointer to get hook to the "connection" or "socket" object */
};


struct http_parser_settings {
  http_cb      on_message_begin;
  http_data_cb on_path;
  http_data_cb on_query_string;
  http_data_cb on_url;
  http_data_cb on_fragment;
  http_data_cb on_header_field;
  http_data_cb on_header_value;
  http_cb      on_headers_complete;
  http_data_cb on_body;
  http_cb      on_message_complete;
};


void http_parser_init(http_parser *parser, enum http_parser_type type);


size_t http_parser_execute(http_parser *parser,
                           const http_parser_settings *settings,
                           const char *data,
                           size_t len);


/* If http_should_keep_alive() in the on_headers_complete or
 * on_message_complete callback returns true, then this will be should be
 * the last message on the connection.
 * If you are the server, respond with the "Connection: close" header.
 * If you are the client, close the connection.
 */
int http_should_keep_alive(http_parser *parser);


#ifdef __cplusplus
}
#endif
#endif
