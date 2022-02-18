#ifndef _ENDPOINTS_
#define _ENDPOINTS_

#include <esp_http_server.h>

#ifdef __cplusplus
extern "C" {
#endif

static esp_err_t serveWifiPageGetHandler(httpd_req_t *req);
static esp_err_t setWifiPostHandler(httpd_req_t *req);
static esp_err_t scanGetHandler(httpd_req_t *req);
static esp_err_t dropGetHandler(httpd_req_t *req);
static esp_err_t serveBusPageGetHandler(httpd_req_t *req);
static esp_err_t setBusPostHandler(httpd_req_t *req);

static const httpd_uri_t setWifi = {
    .uri = "/connection",
    .method = HTTP_POST,
    .handler = setWifiPostHandler,
    .user_ctx = NULL
};

static const httpd_uri_t serveWifiPage = {
    .uri = "/wifi",
    .method = HTTP_GET,
    .handler = serveWifiPageGetHandler,
    .user_ctx = NULL
};

static const httpd_uri_t scan = {
    .uri = "/scan",
    .method = HTTP_GET,
    .handler = scanGetHandler,
    .user_ctx = NULL
};

static const httpd_uri_t drop = {
    .uri = "/drop",
    .method = HTTP_GET,
    .handler = dropGetHandler,
    .user_ctx = NULL
};

static const httpd_uri_t serveBusPage = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = serveBusPageGetHandler,
    .user_ctx = NULL
};

static const httpd_uri_t setBus = {
    .uri = "/setup",
    .method = HTTP_POST,
    .handler = setBusPostHandler,
    .user_ctx = NULL
};

#ifdef __cplusplus
}
#endif

#endif