#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>
#include <http_log.h>

static int helloworld_handler(request_rec *r)
{
    static const char *const helloworld =
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n"
            "<html><head><title>Apache HelloWorld Module</title></head>"
            "<body><h1>Hello World!</h1>"
            "<p>This is the Apache HelloWorld module!</p>"
            "</body></html>";
    apr_status_t rv;
    apr_bucket_brigade *bb;
    apr_bucket *b;
    if (!r->handler || (strcmp(r->handler, "helloworld") != 0)) {
        return DECLINED ;
    }
    if (r->method_number != M_GET) {
        return HTTP_METHOD_NOT_ALLOWED;
    }
    bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    ap_set_content_type(r, "text/html;charset=ascii");

    b = apr_bucket_immortal_create(helloworld, strlen(helloworld), bb->bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(bb, b);
    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_eos_create(bb->bucket_alloc));
    rv = ap_pass_brigade(r->output_filters, bb);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Output Error");
        return HTTP_INTERNAL_SERVER_ERROR;
    }
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
