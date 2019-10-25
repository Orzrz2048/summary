#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>
#include "apr_hash.h"
#include "apr_strings.h"
#include "http_log.h"
#include <ctype.h>
#define MAX_SIZE 8192

static int printitem(void *rec, const char *key, const char *value)
{
    /* rec is a user data pointer. We'll pass the request_rec in it. */
    request_rec *r = rec;
    ap_rprintf(r, "<tr><th scope=\"row\">%s</th><td>%s</td></tr>\n",
                ap_escape_html(r->pool, key),
                ap_escape_html(r->pool, value));
    /* Zero would stop iterating; any other return value continues */
    return 1;
}

static void printtable(request_rec *r, apr_table_t *t,
                    const char *caption, const char *keyhead,
                    const char *valhead)
{
    /* Print a table header */
    ap_rprintf(r, "<table<caption>%s</caption><thead>"
                "<tr><th scope=\"col\">%s</th><th scope=\"col\">%s"
                "</th></tr></thead><tbody>", caption, keyhead, valhead);
    /* Print the data: apr_table_do iterates over entries with
    * our callback
    */
    apr_table_do(printitem, r, t, NULL);
    /* Finish the table */
    ap_rputs("</tbody></table>\n", r);
}

static apr_hash_t *parse_form_from_string(request_rec *r,char *args)
{
    apr_hash_t *form;
    apr_array_header_t *values;
    char *pair;
    char *eq;
    const char *delim = "&";
    char *last;
    char **ptr;
    if (args == NULL) {
        return NULL;
    }

    /* 创建 hash 表*/
    form = apr_hash_make(r->pool);
    /* 将输入的数据，使用 & 进行分割 */
    for (pair = apr_strtok(args, delim, &last);
         pair != NULL;
         pair = apr_strtok(NULL, delim, &last)) {
        for (eq = pair; *eq; ++eq) {
            if (*eq == '+') {
                *eq = ' ';
            }
        }
        /* 根据 = 将key/value分割，并进行转义字符解析 */
        eq = strchr(pair, '=');
        if (eq) {
            *eq++ = '\0';
            ap_unescape_url(pair);
            ap_unescape_url(eq);
        } else {
            eq = "";
            ap_unescape_url(pair);
        }

        /* 将键/值对存放在哈希中。由于同一个键可能有多个值相对应，因此我们将这些值存放
         * 在一个数组中（如果我们碰到该键的第一个值，我们将创建这个数组） */
        values = apr_hash_get(form, pair, APR_HASH_KEY_STRING);
        if (values == NULL) {
            values = apr_array_make(r->pool, 1, sizeof(const char*));
            apr_hash_set(form, pair, APR_HASH_KEY_STRING, values);
        }
        ptr = apr_array_push(values);
        *ptr = apr_pstrdup(r->pool, eq);
    }
    return form;
}

static apr_hash_t* parse_form_from_GET(request_rec *r)
{
    return parse_form_from_string(r, r->args);
}

/* 获取 POST 的数据，假设我们已经对内容类型为 application/x-www-form-urlencoded
 * 进行了调整，假设 *form为空 */
static int parse_form_from_POST(request_rec *r, apr_hash_t **form)
{
    int bytes, eos;
    apr_size_t count;
    apr_status_t rv;
    apr_bucket_brigade *bb;
    apr_bucket_brigade *bbin;
    char *buf;
    apr_bucket *b;
    const char *clen = apr_table_get(r->headers_in, "Content-Length");
    if (clen != NULL) {
        bytes = strtol(clen, NULL, 0);
        if (bytes >= MAX_SIZE) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                          "Request too big (%d bytes; limit %d",
                          bytes, MAX_SIZE);
            return HTTP_REQUEST_ENTITY_TOO_LARGE;
        }
    } else {
        bytes = MAX_SIZE;
    }
    
    bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    bbin = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    count = 0;

    do {
        rv = ap_get_brigade(r->input_filters, bbin, AP_MODE_READBYTES,
                            APR_BLOCK_READ, bytes);
        if (rv != APR_SUCCESS) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r,
                          "failed to read form input");
            return HTTP_INTERNAL_SERVER_ERROR;
        }
        for (b = APR_BRIGADE_FIRST(bbin);
             b != APR_BRIGADE_SENTINEL(bbin);
             b = APR_BUCKET_NEXT(b)) {
            if (APR_BUCKET_IS_EOS(b)) {
                eos = 1;
            }
            if (!APR_BUCKET_IS_METADATA(b)) {
                if (b->length != (apr_size_t)(-1)) {
                    count += b->length;
                    if (count > MAX_SIZE) {
                        /* 我们已经获取了所有数据，现在将这些数据放在缓冲区中
                         * 并对其进行处理 */
                        apr_bucket_delete(b);
                    }
                }
            }
        }
        if (count <= MAX_SIZE) {
            APR_BUCKET_REMOVE(b);
            APR_BRIGADE_INSERT_TAIL(bb, b);
        }
    } while (!eos);
    
    /* 数据处理完毕， 如果我们获得了太多的数据，就kill那个请求 */
    if (count > MAX_SIZE) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r,
                      "Request too big (%d bytes; limit %d)",
                      bytes, MAX_SIZE);
        return HTTP_REQUEST_ENTITY_TOO_LARGE;
    }
    buf = apr_palloc(r->pool, count+1);
    rv = apr_brigade_flatten(bb, buf, &count);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r,
                      "Error (flatten) reading form data");
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    buf[count] = '\0';
    *form = parse_form_from_string(r, buf);

    return OK;
}

static int helloworld_handler(request_rec *r)
{
    apr_hash_t *formdata = NULL;
    int rv = OK;

    if (!r->handler || (strcmp(r->handler, "helloworld") != 0)) {
        return DECLINED ;
    }
    /* 最佳实践：拒绝任何没有显示允许的内容 */
    if (r->method_number != M_GET || r->method_number != M_POST) {
        return HTTP_METHOD_NOT_ALLOWED;
    }
    ap_set_content_type(r, "text/html;charset=ascii");
    ap_rputs("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n"
            "<html><head><title>Apache HelloWorld Module</title></head>"
            "<body><h1>Hello World!</h1>"
            "<p>This is the Apache HelloWorld module!</p>", r);
    /* Print the tables */
    printtable(r, r->headers_in, "Request Headers", "Header", "Value");
    printtable(r, r->headers_out, "Response Headers", "Header", "Value");
    printtable(r, r->subprocess_env, "Environment", "Variable", "Value");

    /* 显示表格中的内容 */
    if (r->method_number == M_GET) {
        formdata = parse_form_from_GET(r);
    } else if (r->method_number == M_POST) {
        const char* ctype = apr_table_get(r->headers_in, "Content-Type");
        if (ctype && (strcasecmp(ctype, "application/x-www-form-urlencoded") == 0)) {
            rv = parse_form_from_POST(r, &formdata);
        }
    }

    if (rv != OK) {
        ap_rputs("<p>Error reading form data!</p>", r);
    } else if (formdata == NULL) {
        ap_rputs("<p>No form data found.</p>", r);
    } else {
        apr_array_header_t *arr;
        char *key;
        apr_ssize_t klen;
        apr_hash_index_t *index;
        char *val;
        char *p;

        ap_rprintf(r, "<h2>Form data supplied by method %s</h2>\n<dl>",
                   r->method);
        for (index = apr_hash_first(r->pool, formdata); index != NULL;
             index = apr_hash_next(index)) {
            apr_hash_this(index, (void**)&key, &klen, (void**)&arr);
            ap_rprintf(r, "<dt>%s</dt>\n", ap_escape_html(r->pool, key));
            for (val = apr_array_pop(arr); val != NULL;
                 val = apr_array_pop(arr)) {
                for (p = val; *p != '\0'; ++p) {
                    if (!isascii(*p)) {
                        *p = '?';
                    }
                }
                ap_rprintf(r, "<dd>%s</dd>\n",
                           ap_escape_html(r->pool, val));
            }
        }
        ap_rputs("</dl>", r);
    }
    ap_rputs("</body></html>", r);
    return OK;
}

static void helloworld_hooks(apr_pool_t *pool)
{
    ap_hook_handler(helloworld_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA helloworld_module = {
    STANDARD20_MODULE_STUFF,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    helloworld_hooks
};