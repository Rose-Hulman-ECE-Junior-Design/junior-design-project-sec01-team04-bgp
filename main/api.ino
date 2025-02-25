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
  this->state->state = State::stopped;
}

static void handle_stop(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  api->stop();
  Serial.println("Stopped!");
  jsonrpc_return_success(r, "{}");
}

CurveData* Api::get_curve(int curve) {
  switch (curve) {
    case 0:
      return &this->state->data.speed;
    case 1:
      return &this->state->data.lookahead_distance;
    case 2:
      return &this->state->data.forward_offset;
    default:
      return nullptr;
  }
}

void Api::set_curve(CurveData* curve_data, double x_start, double x_end, double y_start, double y_end) {
  curve_data->update(x_start, x_end, y_start, y_end);
  this->state->write();
}

static void handle_set_curve(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  double curve, x_start, x_end, y_start, y_end;
  mjson_get_number(r->params, r->params_len, "$[0]", &curve);
  mjson_get_number(r->params, r->params_len, "$[1]", &x_start);
  mjson_get_number(r->params, r->params_len, "$[2]", &x_end);
  mjson_get_number(r->params, r->params_len, "$[3]", &y_start);
  mjson_get_number(r->params, r->params_len, "$[4]", &y_end);

  CurveData* curve_data = api->get_curve((int)curve);
  if (curve_data == nullptr) {
    jsonrpc_return_error(r, 1, "Invalid curve number", "{%Q:%g}", "curve", (int)curve);
    return;
  }

  Serial.printf("Set curve %d\n", (int)curve);
  api->set_curve(curve_data, x_start, x_end, y_start, y_end);
  jsonrpc_return_success(r, "{}");
}

static void handle_get_defaults(struct jsonrpc_request* r) {
  auto api = (Api*)r->userdata;
  double curve;
  mjson_get_number(r->params, r->params_len, "$[0]", &curve);
  CurveData* data = api->get_curve((int)curve);
  if (data == nullptr) {
    jsonrpc_return_error(r, 1, "Invalid curve number", "{%Q:%g}", "curve", (int)curve);
    return;
  }

  // Serial.printf("Sending config defaults: ld = %f, fo = %f, speed = %d\n", data.lookahead_distance, data.forward_offset, data.speed);
  jsonrpc_return_success(r,
                         "{%Q:%g,%Q:%g,%Q:%g,%Q:%g,%Q:%g,%Q:%g,%Q:%g,%Q:%g}",
                         "x_min", data->x_min,
                         "x_start", data->x_start,
                         "x_end", data->x_end,
                         "x_max", data->x_max,
                         "y_min", data->y_min,
                         "y_start", data->y_start,
                         "y_end", data->y_end,
                         "y_max", data->y_max);
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
    while (true)
      ;
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
  jsonrpc_export("set_curve", handle_set_curve);
  jsonrpc_export("get_defaults", handle_get_defaults);
  jsonrpc_export("telemetry", handle_telemetry);
  jsonrpc_export("camera_view", handle_camera_view);

  Serial.println("Setup server callbacks");
  this->server.begin();
  Serial.println("Server started");
}
