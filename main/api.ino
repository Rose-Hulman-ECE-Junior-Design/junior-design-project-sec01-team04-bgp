#include <WiFi.h>
#include <mjson.h>
#include "api.h"
#include "index_html.h"

typedef struct {
    AsyncWebServerRequest* request;
    Api* api;
} Context;

void Api::start() {
    *this->st = VehicleState::started;
}

static void handle_start(struct jsonrpc_request* r) {
    Context* ctx = (Context*)r->userdata;
    ctx->api->start();
    jsonrpc_return_success(r, "{}");
}

void Api::stop() {
    *this->st = VehicleState::stopped;
}

static void handle_stop(struct jsonrpc_request* r) {
    Context* ctx = (Context*)r->userdata;
    ctx->api->stop();
    jsonrpc_return_success(r, "{}");
}

TelemetryData Api::telemetry() {
    return this->tl->read();
}

static void handle_telemetry(struct jsonrpc_request* r) {
    Context* ctx = (Context*)r->userdata;
    TelemetryData data = ctx->api->telemetry();
    jsonrpc_return_success(r, "{%Q:%g,%Q:%g}", "current_ma", data.current_ma, "battery_v", data.battery_v);
}

static int sender(const char* frame, int frame_len, void* ctx) {
    AsyncWebServerRequest* request = ((Context*)ctx)->request;
    AsyncWebServerResponse* response = request->beginResponse_P(200, "application/json-rpc", (uint8_t*)frame, frame_len);
    request->send(response);
}

void Api::init() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConnected");

    Serial.println("Starting server on " + WiFi.softAPIP());

    this->server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", index_html);
    });

    this->server.on("/api", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
      [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        Context ctx = {
            .request = request,
            .api = this,
        };

        jsonrpc_process((char*)data, len, sender, NULL, (void*)&ctx);
    });

    // TODO: Handle GET requests
    // this.server.on("/api", HTTP_GET, [](AsyncWebServerRequest* request) {
    //
    // });

    this->server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404);
    });

    jsonrpc_init(NULL, NULL);
    jsonrpc_export("start", handle_start);
    jsonrpc_export("stop", handle_stop);
    jsonrpc_export("telemetry", handle_telemetry);
    
    Serial.println("Setup server callbacks");
    this->server.begin();
    Serial.println("Server started");
}
