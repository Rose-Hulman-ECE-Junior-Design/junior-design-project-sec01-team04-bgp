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
  constructor(canvas, x_min, x_max, y_min, y_max) {
    this.canvas = canvas;
    this.x_min = x_min;
    this.x_max = x_max;
    this.y_min = y_min;
    this.y_max = y_max;

    this.point1 = { x: x_min, y: y_min };
    this.point2 = { x: x_max, y: y_max };
  }

  draw() {
    var canvas = this.canvas;
    var ctx = canvas.getContext('2d');

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    drawPoint(this.point1);
    drawPoint(this.point2);

    const start = { x: x_min, y: y_min };
    const end = { x: x_max, y: y_max };
    drawLine(start, this.point1);
    drawLine(this.point1, this.point2);
    drawLine(this.point2, end);
  }

  toCanvasCoords(point) {
    point.x -= this.x_min;
    point.x *= this.canvas.width / (this.x_max - this.x_min);
    point.y -= this.y_min;
    point.y *= this.canvas.height / (this.y_max - this.y_min);
    return point;
  }

  drawPoint(point) {
    const point_size = 5;
    const point = toCanvasCoords(point);

    ctx.beginPath();
    ctx.arc(point.x, point.y, point_size, 0, 2 * Math.PI);
    ctx.fillStyle = '#00000080';
    ctx.fill();
    ctx.lineWidth = 2;
    ctx.strokeStyle = '#000000';
    ctx.stroke();
  }

  drawLine(point1, point2) {
    const point1 = toCanvasCoords(point1);
    const point2 = toCanvasCoords(point2);

    ctx.beginPath();
    ctx.moveTo(point1.x, point1.y);
    ctx.lineTo(point2.x, point2.y);
    ctx.lineWidth = 2;
    ctx.strokeStyle = '#000000';
    ctx.stroke();
  }
}
