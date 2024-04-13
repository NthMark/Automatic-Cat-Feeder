#include <ESP32Servo.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "HX711.h"
#include <LiquidCrystal_I2C.h>
#include <DHT.h>    
Servo myservo; 
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
LiquidCrystal_I2C lcd1(0x26, 16, 2);

//Data communication
char c;
String dataIn;

//DHT11
const int DHTPIN = 8;
const int DHTTYPE = DHT11;
DHT dht(DHTPIN, DHTTYPE);

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
int pos = 0;
int thresholdWeight=0;
int thresholdTimer=0;
HX711 scale;

String message="";
// Replace with your network credentials
const char* ssid = "Thien Tu";
const char* password = "thienhao";

//Time
unsigned long timedelay;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 2rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: rgb(15, 233, 19);
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
   label {
      display: block;
      margin-bottom: 8px;
      text-align: center;
      font-size: 20px;
      
    }

    input {
      width: 100%;
      padding: 8px;
      box-sizing: border-box;
      border: 1px solid #ccc;
      border-radius: 4px;
      font-size: 16px;
    }
    input::-webkit-outer-spin-button,
input::-webkit-inner-spin-button {
  -webkit-appearance: none;
  margin: 0;
}
   select {
  /* Reset */
  appearance: none;
  border: 0;
  outline: 0;
  font: inherit;
  /* Personalize */
  margin: 0 0 10px 0;
  width: 20rem;
  padding: 1rem 4rem 1rem 1rem;
  background: var(--arrow-icon) no-repeat right 0.8em center / 1.4em,
    linear-gradient(to left, var(--arrow-bg) 3em, var(--select-bg) 3em);
  color: black;
  border-radius: 0.25em;
  box-shadow: 0 0 1em 0 rgba(0, 0, 0, 0.2);
  cursor: pointer;
  /* Remove IE arrow */
  &::-ms-expand {
    display: none;
  }
  /* Remove focus outline */
  &:focus {
    outline: none;
  }
  /* <option> colors */
  option {
    color: inherit;
    background-color: var(--option-bg);
  }
}
.footer-distributed {
  background-color: #292c2f;
  box-shadow: 0 1px 1px 0 rgba(0, 0, 0, 0.12);
  box-sizing: border-box;
  width: 100%;
  text-align: left;
  font: normal 16px sans-serif;
  padding: 45px 50px;
}

.footer-distributed .footer-left p {
  color: #8f9296;
  font-size: 14px;
  margin: 0;
}
/* Footer links */

.footer-distributed p.footer-links {
  font-size: 18px;
  font-weight: bold;
  color: #ffffff;
  margin: 0 0 10px;
  padding: 0;
  transition: ease .25s;
}

.footer-distributed p.footer-links a {
  display: inline-block;
  line-height: 1.8;
  text-decoration: none;
  color: inherit;
  transition: ease .25s;
}

.footer-distributed .footer-links a:before {
  content: "·";
  font-size: 20px;
  left: 0;
  color: #fff;
  display: inline-block;
  padding-right: 5px;
}

.footer-distributed .footer-links .link-1:before {
  content: none;
}

.footer-distributed .footer-right {
  float: right;
  margin-top: 6px;
  max-width: 180px;
}

.footer-distributed .footer-right a {
  display: inline-block;
  width: 35px;
  height: 35px;
  background-color: #33383b;
  border-radius: 2px;
  font-size: 20px;
  color: #ffffff;
  text-align: center;
  line-height: 35px;
  margin-left: 3px;
  transition:all .25s;
}

.footer-distributed .footer-right a:hover{transform:scale(1.1); -webkit-transform:scale(1.1);}

.footer-distributed p.footer-links a:hover{text-decoration:underline;}

/* Media Queries */

@media (max-width: 600px) {
  .footer-distributed .footer-left, .footer-distributed .footer-right {
    text-align: center;
  }
  .footer-distributed .footer-right {
    float: none;
    margin: 0 auto 20px;
  }
  .footer-distributed .footer-left p.footer-links {
    line-height: 1.8;
  }
}
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>Cat food dispenser</h1>
  </div>

  <div class="content">
    <div class="card">
      <h2>FOOD FEEDING CONTROLLER</h2>
    <label for="cars">Choose time interval for feeding cat:</label>
  <select name="interval" id="interval">
    <option selected value="12">12 hours</option>
    <option value="8">8 hours</option>
    <option value="6">6 hours</option>
    <option value="4">4 hours</option>
  </select>
  <label for="weight">Enter amount of weight:</label>
  <input type="number" id="weight" placeholder="Enter amount of weight (gam)">
      <p><button id="button" class="button" onclick="sendValues()">Submit</button></p>
    </div>
  </div>

  <footer class="footer-distributed">
    <div class="footer-left">
        <p>Nhom 3- Nguyen Thien Hao, Dao Ngoc Duc va Duong Ba Hung</p>
    </div>

</footer>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function getValue() {
    websocket.send('getValue');
  }
  function onOpen(event) {
    console.log('Connection opened');
    getValue();
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "getValue"){
        document.getElementById('weight').value=0;
    }
  }
  function onLoad(event) {
    initWebSocket();
  }
  function sendValues() {
    var weightInput=document.getElementById('weight').value;
    var intervalInput = document.getElementById('interval').value;
    // if(weightInput>0){
    websocket.send(weightInput.toString()+","+ intervalInput.toString());
    
  }

</script>
</body>
</html>
)rawliteral";

void notifyClients() {
  ws.textAll("getValue");
}
String tempWeight="--";
String tempInterval="++";
int isUpdate=0;
String previousMessage="";
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message=(char*)data;
    int idx=message.indexOf(",");
    tempWeight=message.substring(0,idx);
    thresholdWeight=tempWeight.toInt();
    tempInterval=message.substring(idx+1);
    thresholdTimer=tempInterval.toInt();
    if (strcmp((char*)data, "getValue") == 0) {
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

unsigned long timedelay1;
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial2.begin(9600);
  timedelay=millis();
  timedelay1=millis();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Start server
  server.begin();
}
String correctMessage="";
void loop() {
  
//  while(Serial2.available()){
//    c=Serial2.read();
//    if(c!='\n'){
//      dataIn+=c;
//    }
//    else{
//      break;
//    }
//  }
//  if(c=='\n'){
//    Serial.println(dataIn);
//    c=0;
//    dataIn="";
//  }
  if((unsigned long)(millis()-timedelay)>2000){
    if (strcmp(message.c_str(), previousMessage.c_str()) != 0) {
      previousMessage=message;
      correctMessage="1,"+previousMessage;
    }
    else{
      correctMessage="0,"+previousMessage;
  }
  Serial2.print(correctMessage+"\n");
    Serial.println("message:"+message);
    Serial.println("prev_message:"+previousMessage);
    Serial.println(thresholdWeight);
    Serial.println(thresholdTimer);
    Serial.println(correctMessage);
      timedelay=millis();
  }
  
  ws.cleanupClients();
}
