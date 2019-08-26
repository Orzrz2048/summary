#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>
#include <http_log.h>

#define BUFLEN 8192
static int check_postdata_old_method(request_rec *r)
{
    char buf[BUFLEN];
    size_t bytes, count = 0;

    if (ap_setup_client_block(r, REQUEST_CHUNKED_DECHUNK) != OK) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "Bad request body!");
        ap_rputs("<p>Bad request body.</p>\n", r);
        return HTTP_BAD_REQUEST;
    }
    if (ap_should_client_block(r)) {
        for (bytes = ap_get_client_block(r, buf, BUFLEN); bytes > 0;
             bytes = ap_get_client_block(r, buf, BUFLEN))
        {
            count += bytes;
        }
        ap_rprintf(r, "<p>Got %ld bytes of request body data.</p>\n", count);
    } else {
        ap_rputs("<p>No request body.</p>", r);
    }
    return OK;
}

static int check_postdata_new_method(request_rec *r)
{
    apr_status_t status;
    int end = 0;
    apr_size_t bytes, count = 0;
    const char *buf;
    apr_bucket *b;
    apr_bucket_brigade *bb;

    /* 检查是否还有需要读入的输入。
     * 客户端可通过 Content-Length 或 Transfer-Encoding 告诉我们。
     */
    int has_input = 0;
    const char *hdr = apr_table_get(r->headers_in, "Content-Length");
    if (hdr) {
        has_input = 1;
    }
    hdr = apr_table_get(r->headers_in, "Transfer-Encoding");
    if (hdr) {
        if (strcasecmp(hdr, "chunked") == 0) {
            has_input = 1;
        } else {
            ap_rprintf(r, "</>Unsupported Transfer Encoding: %s</p>",
                       ap_escape_html(r->pool, hdr));
            return OK;
        }
    }
    if (!has_input) {
        ap_rputs("<p>No request body.</p>\n", r);
        return OK;
    }

    /* 读取数据并统计 */
    /* 创建 APR 的 Brigade 数据 */
    bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    /* 循环执行，直到到达队列结束 */
    do
    {
        /* 读取一个输入的程序块到bb */
        status = ap_get_brigade(r->input_filters, bb, AP_MODE_READBYTES, APR_BLOCK_READ, BUFLEN);
        if (status == APR_SUCCESS)
        {
            /* 循环执行 */
            for (b = APR_BRIGADE_FIRST(bb);
                 b != APR_BRIGADE_SENTINEL(bb);
                 b = APR_BUCKET_NEXT(b))
            {
                /* Check for EOS */
                if (APR_BUCKET_IS_EOS(b)) {
                    end = 1;
                    break;
                } else if (APR_BUCKET_IS_METADATA(b)) {
                    continue;
                }
                bytes = BUFLEN;
                status = apr_bucket_read(b, &buf, &bytes, APR_BLOCK_READ);
                count += bytes;
            }
        }
        /* 结束时删除数据 */
        apr_brigade_cleanup(bb);
    } while (!end && (status == APR_SUCCESS));

    if (status == APR_SUCCESS) {
        ap_rprintf(r, "<p>Got %ld bytes of request body data.</p>\n",count);
        return OK;
    } else {
        ap_rputs("<p>Error reading request body.</p>", r);
        return OK;
    }
}

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

static int helloworld_handler(request_rec *r)
{
    if (!r->handler || (strcmp(r->handler, "helloworld") != 0)) {
        return DECLINED ;
    }
    if (r->method_number != M_GET) {
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
    check_postdata_old_method(r);
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
