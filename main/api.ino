#include <WiFi.h>
#include <mjson.h>
#include "api.h"

void Api::start() {
  this->state->state = State::started;
}

static void handle_start(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  api->start();
  Serial.println("Started!");
  jsonrpc_return_success(r, "{}");
}

void Api::stop() {
  this->state->state = State::started;
}

static void handle_stop(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  api->stop();
  Serial.println("Stopped!");
  jsonrpc_return_success(r, "{}");
}

void Api::set_speed(int speed) {
  this->state->data.speed = constrain(speed, 0, 150);
  this->state->write();
}

static void handle_set_speed(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  double speed;
  mjson_get_number(r->params, r->params_len, "$[0]", &speed);
  Serial.printf("Set speed to %d\n", speed);
  api->set_speed(speed);
  jsonrpc_return_success(r, "{}");
}

void Api::set_lookahead_distance(double distance) {
  this->state->data.lookahead_distance = max(distance, 0.0);
  this->state->write();
}

static void handle_set_lookahead_distance(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  double distance;
  mjson_get_number(r->params, r->params_len, "$[0]", &distance);
  Serial.printf("Set lookahead distance to %f\n", distance);
  api->set_lookahead_distance(distance);
  jsonrpc_return_success(r, "{}");
}

void Api::set_forward_offset(double offset) {
  this->state->data.forward_offset = max(offset, 0.0);
  this->state->write();
}

static void handle_set_forward_offset(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  double forward_offset;
  mjson_get_number(r->params, r->params_len, "$[0]", &forward_offset);
  Serial.printf("Set forward offset to %f\n", forward_offset);
  api->set_forward_offset(forward_offset);
  jsonrpc_return_success(r, "{}");
}

Data Api::get_defaults() {
  return this->state->data;
}

static void handle_get_defaults(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  Data data = api->get_defaults();
  Serial.printf("Sending config defaults: ld = %f, fo = %f, speed = %d\n", data.lookahead_distance, data.forward_offset, data.speed);
  jsonrpc_return_success(r, "{%Q:%g,%Q:%g,%Q:%g}", "speed", (double)data.speed, "lookahead_distance", data.lookahead_distance, "forward_offset", data.forward_offset);
}

TelemetryData Api::telemetry() {
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

  Serial.println("Initializing server callbacks");

  this->server.serveStatic("/", this->fs, "/www/").setDefaultFile("index.html");

  this->server.on(
    "/api", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL,
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
      char* response = nullptr;
      jsonrpc_process((const char*)data, len, mjson_print_dynamic_buf, &response, (void*)this);
      request->send(200, "application/json-rpc", response);
      free(response);
    });

  this->server.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404);
  });

  jsonrpc_init(NULL, NULL);
  jsonrpc_export("start", handle_start);
  jsonrpc_export("stop", handle_stop);
  jsonrpc_export("set_speed", handle_set_speed);
  jsonrpc_export("set_lookahead_distance", handle_set_lookahead_distance);
  jsonrpc_export("set_forward_offset", handle_set_forward_offset);
  jsonrpc_export("get_defaults", handle_get_defaults);
  jsonrpc_export("telemetry", handle_telemetry);
  jsonrpc_export("camera_view", handle_camera_view);

  Serial.println("Setup server callbacks");
  this->server.begin();
  Serial.println("Server started");
}
