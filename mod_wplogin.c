#include <stdio.h>
#include "apr_hash.h"
#include "ap_config.h"
#include "ap_provider.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"

static int  wplogin_handler(request_rec *r);
static int  wplogin_is_authorized(request_rec *r);
static void register_hooks(apr_pool_t *pool);

module AP_MODULE_DECLARE_DATA    wplogin_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,               /* Per-directory configuration handler */
    NULL,               /* Merge handler for per-directory configurations */
    NULL,               /* Per-server configuration handler */
    NULL,               /* Merge handler for per-server configurations */
    NULL,               /* Any directives we may have for httpd */
    register_hooks      /* Our hook registering function */
};

static void register_hooks(apr_pool_t *pool)
{
    ap_hook_handler(wplogin_handler, NULL, NULL, APR_HOOK_FIRST);
}

static int wplogin_handler(request_rec *r)
{
    /* Skip all requests where the file name is not wp-login.php */
    if (NULL == strstr(r->filename, "wp-login.php")) {
        return DECLINED;
    }

    int  authorized;
    char uri[255];

    authorized = wplogin_is_authorized(r);

    // Already have auth cookie, so good to go
    if (1 == authorized) {
        return DECLINED;
    }

    // POST with no auth cookie in query string not allowed
    if (NULL != strstr(r->method, "POST")) {
        return HTTP_UNAUTHORIZED;
    }

    // When auth-cookie is present in query string, return JS to set cookie and redirect back to wp-login
    if (NULL != r->args && NULL != strstr(r->args, "auth-cookie")) {
        ap_set_content_type(r, "text/html");
        ap_rprintf(r, "<html>");
        ap_rprintf(r, "<head>");
        ap_rprintf(r, "<script type=\"text/javascript\">");
        ap_rprintf(r, "document.cookie=\"wplogin=authorized; expires=0\";");
        ap_rprintf(r, "window.location=\"%s\";", r->uri);
        ap_rprintf(r, "</script>");
        ap_rprintf(r, "</head>");
        ap_rprintf(r, "</html>");
        return OK;

    // Redirect to auth-cookie when not authorized
    } else {
        snprintf(uri, 255, "%s?auth-cookie", r->uri);
        apr_table_set(r->headers_out, "Location", uri);
        return HTTP_MOVED_TEMPORARILY;
    }
}

static int wplogin_is_authorized(request_rec *r)
{
    const apr_array_header_t *fields;
    int                      i;
    int                      authorized;
    apr_table_entry_t        *e = 0;

    fields     = apr_table_elts(r->headers_in);
    e          = (apr_table_entry_t *) fields->elts;
    authorized = 0;

    for (i = 0; i < fields->nelts; i++) {
        if (NULL != strstr(e[i].key, "Cookie") && NULL != strstr(e[i].val, "wplogin=authorized")) {
            authorized = 1;
        }
    }

    return authorized;
}
