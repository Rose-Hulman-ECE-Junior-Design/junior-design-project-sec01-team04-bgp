"use strict";

const api_url = '/api';
var i = 1;

var telemetry_timeout = 1000; 
var telemetry_enabled = true;
var camera_timeout = 1000; 

function json_rpc_call(method, params, callback) {
  var xhr = new XMLHttpRequest();

  xhr.onreadystatechange = function () {
    if (xhr.readyState === 4 && xhr.status === 200) {
      callback(JSON.parse(xhr.responseText).result);
    }
  };

  var data = JSON.stringify({
    "jsonrpc": "2.0",
    "method": method,
    "params": params,
    "id": i++,
  });

  xhr.open('POST', api_url, true);
  xhr.setRequestHeader('Content-Type', 'application/json-rpc');
  xhr.send(data);
}

var CsvWriter = function() {
  this.parts = [];
};

CsvWriter.prototype.append = function(row) {
  this.parts.push(row.join(',') + '\n');
};

CsvWriter.prototype.download = function() {
  const data = new Blob(this.parts, { type: 'text/plain' });
  const data_url = window.URL.createObjectURL(data);
  const link = document.getElementById('download_link');
  link.href = data_url;
  link.click();
}

const labels = ['Voltage (V)', 'Current (A)', 'Power (W)'];
const backgroundColors = ['#36a2eb80', '#ffcd5680', '#4bc0c080'];
const borderColors = ['#36a2eb', '#ffcd56', '#4bc0c0'];

var chart = new Chart(document.getElementById('chart'), {
  type: 'line',
  data: {
    labels: [],
    datasets: labels.map((label, i) => ({
      label: label,
      data: [],
      backgroundColor: backgroundColors[i],
      borderColor: borderColors[i],
    })),
  },
  options: {
    responsive: true,
  },
});

function add_data(values) {
  values.forEach((value, i) => {
    chart.data.datasets[i].data.push(value);
  });

  chart.data.labels.push(new Date().toLocaleTimeString());
  chart.update();
}

function clear_fn() {
  chart.data.datasets.forEach(dataset => { dataset.data.length = 0; });
  chart.update();
}

var download = document.getElementById('download');

download.onclick = function() {
  let rows = chart.data.datasets[0].data.map((_, colIndex) =>
      chart.data.datasets.map(row => row.data[colIndex])
  );

  var writer = new CsvWriter();
  writer.append(labels);
  for (row of rows) {
    writer.append(row);
  }

  writer.download();
};

var start = document.getElementById('start');
var stop = document.getElementById('stop');
var timer_value = document.getElementById('timer_value');
var timed_stop = document.getElementById('timed_stop');
var motor_speed = document.getElementById('motor_speed');
var camera_refresh_rate = document.getElementById('telemetry_refresh_rate');
var lookahead_distance = document.getElementById('lookahead_distance');
var forward_offset = document.getElementById('forward_offset');

var enable = document.getElementById('enable');
var disable = document.getElementById('disable');
var clear = document.getElementById('clear');
var telemetry_refresh_rate = document.getElementById('telemetry_refresh_rate');

start.onclick = function() {
  start.disabled = true;
  stop.disabled = false;
  timed_stop.disabled = false;

  json_rpc_call('start', [], () => {
    console.log("Started vehicle");
  });
};

function stop_fn() {
  start.disabled = false;
  stop.disabled = true;
  timed_stop.disabled = true;

  json_rpc_call('stop', [], () => {
    console.log("Stopped vehicle");
  });
}

stop.onclick = stop_fn;
timed_stop.onclick = function() {
  timed_stop.disabled = true;
  window.setTimeout(stop_fn, parseInt(timer_value.value) * 1000);
};

motor_speed.addEventListener('change', function() {
  const speed = parseInt(this.value, 10);
  json_rpc_call('set_speed', [speed], () => {
    console.log("Set speed to " + speed);
  });
});

lookahead_distance.addEventListener('change', function() {
  const distance = parseInt(this.value, 10);
  json_rpc_call('set_lookahead_distance', [distance], () => {
    console.log("Set lookahead distance to " + distance);
  });
});

forward_offset.addEventListener('change', function() {
  const forward_offset = parseInt(this.value, 10);
  json_rpc_call('set_forward_offset', [forward_offset], () => {
    console.log("Set forward offset to " + forward_offset);
  });
});

// On startup, get default config values from vehicle
json_rpc_call('get_defaults', [], (res) => {
  console.log(res);
  forward_offset.value = res.forward_offset.toString();
  lookahead_distance.value = res.lookahead_distance.toString();
  motor_speed.value = res.speed.toString();
  console.log("Set default config values");
});

enable.onclick = function() {
  telemetry_enabled = true;
  enable.disabled = true;
  disable.disabled = false;
}

disable.onclick = function() {
  telemetry_enabled = false;
  enable.disabled = false;
  disable.disabled = true;
}

clear.onclick = clear_fn;

telemetry_refresh_rate.addEventListener('change', function() {
  telemetry_timeout = parseInt(this.value, 10);
});

function get_telemetry() {
  if (telemetry_enabled) {
    json_rpc_call('telemetry', [], (res) => {
      add_data([res.battery_v, res.current_a, res.power_w]);
    });
  }

  window.setTimeout(get_telemetry, telemetry_timeout);
}

window.setTimeout(get_telemetry, telemetry_timeout);

function draw_camera_view(info) {
  var canvas = document.getElementById('camera_view');
  var ctx = canvas.getContext('2d');

  ctx.clearRect(0, 0, canvas.width, canvas.height);

  ctx.beginPath();
  ctx.moveTo(info.origin.x, canvas.height - info.origin.y);
  ctx.lineTo(info.target.x, canvas.height - info.target.y);

  ctx.lineWidth = 2;
  ctx.strokeStyle = 'red';
  ctx.stroke();
}

camera_refresh_rate.addEventListener('change', function() {
  camera_timeout = parseInt(this.value, 10);
});

function get_camera_view() {
  json_rpc_call('camera_view', [], (res) => {
    draw_camera_view(res);
  });

  window.setTimeout(get_camera_view, camera_timeout);
}

window.setTimeout(get_camera_view, camera_timeout);


class CurveSelector {
  constructor(opts) {
    this.canvas = opts.canvas;
    this.curve_number = opts.curve_number;
    this.x_label = opts.x.label;
    this.y_label = opts.y.label;

    json_rpc_call('get_defaults', [this.curve_number], (res) => {
      this.x_min = res.x_min;
      this.x_max = res.x_max;
      this.y_min = res.y_min;
      this.y_max = res.y_max;

      this.point1 = { x: res.x_start, y: res.y_start };
      this.point2 = { x: res.x_end, y: res.y_end };
    });

    this.onchange = () => {
      json_rpc_call('set_curve', [this.curve_number, this.point1.x, this.point2.x, this.point1.y, this.point2.y], () => {
        console.log("Set curve " + this.curve_number);
      };
    };

    var drag_point = null;
    this.canvas.onmousedown = async (e) => {
      await this.canvas.requestPointerLock();

      if (this.hits(this.point1, e)) {
        drag_point = this.point1;
      } else if (this.hits(this.point2, e)) {
        drag_point = this.point2;
      }
    };

    this.canvas.onmousemove = (e) => {
      if (drag_point == null) return;

      const move = this.scaleCanvasMovement(e);
      drag_point.x += move.x;
      drag_point.y += move.y;
      this.constrainPoint(drag_point);
      this.draw();
      this.drawLabel(drag_point);
    }

    this.canvas.onmouseup = (e) => {
      document.exitPointerLock();

      if (drag_point != null) {
        drag_point = null;
        this.draw();
        this.onchange();
      }
    }

    this.draw();
  }

  getPosition(event) {
      const rect = this.canvas.getBoundingClientRect();
      const x = event.clientX - rect.left;
      const y = event.clientY - rect.top;
      return { x: x, y: y };
  }

  toCanvasCoords(point) {
    const new_x = (point.x - this.x_min) * this.canvas.width / (this.x_max - this.x_min);
    const new_y = (this.y_max - point.y) * this.canvas.height / (this.y_max - this.y_min);
    return { x: new_x, y: new_y };
  }

  scaleCanvasMovement(e) {
    const new_x = e.movementX * (this.x_max - this.x_min) / this.canvas.width;
    const new_y = -e.movementY * (this.y_max - this.y_min) / this.canvas.height;
    return { x: new_x, y: new_y };
  }

  constrainPoint(point) {
    // Points must be within bounds
    point.x = Math.max(Math.min(point.x, this.x_max), this.x_min);
    point.y = Math.max(Math.min(point.y, this.y_max), this.y_min);
    // Point can't go beyond other point
    // (That would create a curve that isn't a one-to-one function)
    if (point.x == this.point1.x) {
      point.x = Math.min(point.x, this.point2.x);
    } else {
      point.x = Math.max(point.x, this.point1.x);
    }
  }

  hits(point, e) {
    const hit_radius = 20; // Radius to detect hit in px
    const point1 = this.toCanvasCoords(point);
    const point2 = this.getPosition(e);
    return Math.sqrt((point2.x - point1.x) ** 2 + (point2.y - point1.y) ** 2) < hit_radius;
  }

  draw() {
    var canvas = this.canvas;
    var ctx = canvas.getContext('2d');

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // ctx.scale(0.9, 0.9);
    // ctx.translate(0.1 * this.canvas.width, -0.1 * this.canvas.height);

    this.drawPoint(this.point1);
    this.drawPoint(this.point2);

    const start = { x: this.x_min, y: this.point1.y };
    const end = { x: this.x_max, y: this.point2.y };
    this.drawLine(start, this.point1);
    this.drawLine(this.point1, this.point2);
    this.drawLine(this.point2, end);

    // ctx.translate(-0.1 * this.canvas.width, 0.1 * this.canvas.height);
    // ctx.resetTransform();

    // this.drawAxis();
  }

  drawPoint(point) {
    const point_size = 5;
    const real_point = this.toCanvasCoords(point);
    var ctx = this.canvas.getContext('2d');

    ctx.beginPath();
    ctx.arc(real_point.x, real_point.y, point_size, 0, 2 * Math.PI);
    ctx.fillStyle = '#00000080';
    ctx.fill();
    ctx.lineWidth = 2;
    ctx.strokeStyle = '#000000';
    ctx.stroke();
    ctx.closePath();
  }

  drawLine(point1, point2) {
    const real_point1 = this.toCanvasCoords(point1);
    const real_point2 = this.toCanvasCoords(point2);
    var ctx = this.canvas.getContext('2d');

    ctx.beginPath();
    ctx.moveTo(real_point1.x, real_point1.y);
    ctx.lineTo(real_point2.x, real_point2.y);
    ctx.lineWidth = 2;
    ctx.strokeStyle = '#000000';
    ctx.stroke();
    ctx.closePath();
  }

  drawLabel(point) {
    var ctx = this.canvas.getContext('2d');

    const real_point = this.toCanvasCoords(point);
    const line_height = 15;
    const line1 = `${this.x_label}: ${point.x.toPrecision(4)}`;
    const line2 = `${this.y_label}: ${point.y.toPrecision(4)}`;
    const text_width = Math.max(ctx.measureText(line1).width, ctx.measureText(line2).width);
    const text_x = Math.min(real_point.x, this.canvas.width - text_width);
    const text_y = Math.max(real_point.y, 2 * line_height);

    ctx.font = "12px Arial";
    ctx.fillStyle = "#ffffff";
    ctx.shadowColor = "#000000";
    ctx.textBaseline = "bottom";
    ctx.lineWidth = 3;
    ctx.strokeText(line1, text_x, text_y);
    ctx.strokeText(line2, text_x, text_y - line_height);
    ctx.fillText(line1, text_x, text_y);
    ctx.fillText(line2, text_x, text_y - line_height);
  }

  drawAxis() {
    var ctx = this.canvas.getContext('2d');
    // ctx.scale(0.9, 0.9);
    ctx.translate(0.1 * this.canvas.width, -0.1 * this.canvas.height);
  }
}

var speed_curve = new CurveSelector({
  canvas: document.getElementById('speed_graph'),
  curve_number: 0,
  x_label: 'Steering Angle (deg)',
  y_label: 'Speed',
});

var lookahead_distance_curve = new CurveSelector({
  canvas: document.getElementById('lookahead_distance_graph'),
  curve_number: 1,
  x_label: 'Steering Angle (deg)',
  y_label: 'Lookahead Distance',
});

var forward_offset_curve = new CurveSelector({
  canvas: document.getElementById('forward_offset_graph'),
  curve_number: 2,
  x_label: 'Steering Angle (deg)',
  y_label: 'Forward Offset',
});
