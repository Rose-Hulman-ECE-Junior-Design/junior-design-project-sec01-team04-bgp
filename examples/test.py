# This example script requires the jsonrpcclient library
# It can be installed by running the following command:
# `pip install "jsonrpcclient[requests]"`
from jsonrpcclient import request

# API endpoint. See docs for how to find this
url = "http://192.168.4.1/api"

# Send a request. Pass the API's url, the method name, and the arguments 
request(url, "start")

request(url, "set_speed", 40)

request(url, "stop")
