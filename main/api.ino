#include <WiFi.h>
#include <mjson.h>
#include <LittleFS.h>
#include "api.h"
#include "state.h"

static void handle_start(struct jsonrpc_request* r) {
  state = VehicleState::started;
  Serial.println("Started!");
  jsonrpc_return_success(r, "{}");
}

static void handle_stop(struct jsonrpc_request* r) {
  state = VehicleState::stopped;
  Serial.println("Stopped!");
  jsonrpc_return_success(r, "{}");
}

static void handle_set_speed(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  double new_speed;
  mjson_get_number(r->params, r->params_len, "$[0]", &new_speed);
  speed = constrain((int)new_speed, 0, 150);
  Serial.printf("Set speed to %d\n", speed);
  jsonrpc_return_success(r, "{}");
}

static void handle_set_lookahead_distance(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  mjson_get_number(r->params, r->params_len, "$[0]", &lookahead_distance);
  Serial.printf("Set lookahead_distance to %d\n", lookahead_distance);
  jsonrpc_return_success(r, "{}");
}

static void handle_set_forward_offset(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  mjson_get_number(r->params, r->params_len, "$[0]", &forward_offset);
  Serial.printf("Set forward offset to %d\n", forward_offset);
  jsonrpc_return_success(r, "{}");
}

TelemetryData Api::telemetry() {
  // Serial.println("Sending telemetry");
  return this->tl->read();
}

static void handle_telemetry(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  TelemetryData data = api->telemetry();
  jsonrpc_return_success(r, "{%Q:%g,%Q:%g,%Q:%g}", "current_a", data.current_a, "battery_v", data.battery_v, "power_w", data.power_w);
}

CameraView Api::camera_view() {
  return this->camera->get_camera_view();
}

static void handle_camera_view(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  CameraView view = api->camera_view();
  jsonrpc_return_success(r, "{%Q:{%Q:%g,%Q:%g},%Q:{%Q:%g,%Q:%g}}", "target", "x", view.target.x, "y", view.target.y, "origin", "x", view.origin.x, "y", view.origin.y);
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

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  Serial.println("Initialized filesystem");

  Serial.println("Initializing server callbacks");

  this->server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");

  this->server.on("/api", HTTP_POST, [](AsyncWebServerRequest* request){}, NULL,
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
      char* response = nullptr;
      jsonrpc_process((const char*)data, len, mjson_print_dynamic_buf, &response, (void*)this);
      request->send(200, "application/json-rpc", response);
      free(response);
  });

  this->server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  jsonrpc_init(NULL, NULL);
  jsonrpc_export("start", handle_start);
  jsonrpc_export("stop", handle_stop);
  jsonrpc_export("set_speed", handle_set_speed);
  jsonrpc_export("set_lookahead_distance", handle_set_lookahead_distance);
  jsonrpc_export("set_forward_offset", handle_set_forward_offset);
  jsonrpc_export("telemetry", handle_telemetry);
  jsonrpc_export("camera_view", handle_camera_view);
  
  Serial.println("Setup server callbacks");
  this->server.begin();
  Serial.println("Server started");
}
