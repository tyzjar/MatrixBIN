
//var mqtt = { hostname: "test.mosquitto.org", port: 8080 };
var mqtt = { hostname: "192.168.56.101", port: 8080 };
//var topic_name = "world";
var topic_name = "temperature/celsius";
var topic_name_2 = "temperature/fahrenheit";
var publish_cnt = 0;

// called when the client loses its connection
function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost: "+responseObject.errorMessage);
  }
}

// called when a message arrives
function onMessageArrived(message) {
  console.log("onMessageArrived: "+ message.topic + " = "  + message.payloadString);
}
