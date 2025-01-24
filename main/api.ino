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
    Serial.println("Started!");
}

static void handle_start(struct jsonrpc_request* r) {
    Context* ctx = (Context*)r->userdata;
    ctx->api->start();
    jsonrpc_return_success(r, "{}");
}

void Api::stop() {
    *this->st = VehicleState::stopped;
    Serial.println("Stopped!");
}

static void handle_stop(struct jsonrpc_request* r) {
    Context* ctx = (Context*)r->userdata;
    ctx->api->stop();
    jsonrpc_return_success(r, "{}");
}

TelemetryData Api::telemetry() {
    Serial.println("Sending telemetry");
    return this->tl->read();
}

static void handle_telemetry(struct jsonrpc_request* r) {
    Context* ctx = (Context*)r->userdata;
    TelemetryData data = ctx->api->telemetry();
    jsonrpc_return_success(r, "{%Q:%g,%Q:%g,%Q:%g}", "current_ma", data.current_ma, "battery_v", data.battery_v, "power_mw", data.power_mw);
}

void Api::init() {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(ssid, password)) {
      Serial.println("WiFi AP failed");
      while (true);
    }
    delay(1000);

    Serial.print("Starting server on ");
    Serial.println(WiFi.softAPIP());

    Serial.println("Initializing server callbacks");
    this->server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        Serial.println("Received GET request");
        request->send(200, "text/html", index_html);
        Serial.println("Handled GET request");
    });

    this->server.on("/api", HTTP_POST, [](AsyncWebServerRequest* request){}, NULL,
      [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        Serial.println("Received POST request");
        Context ctx = {
            .request = request,
            .api = this,
        };

        // jsonrpc_process((const char*)data, len, sender, NULL, (void*)ctx);
        char* response = nullptr;
        jsonrpc_process((const char*)data, len, mjson_print_dynamic_buf, &response, (void*)&ctx);
        request->send(200, "application/json-rpc", response);
        // AsyncWebServerResponse* response = request->beginResponse_P(200, "application/json-rpc", (const uint8_t*)frame, strlen(frame));
        // request->send(response);
        free(response);
        Serial.println("Handled POST request");
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
