#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>

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