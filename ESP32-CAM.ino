//#include "WifiCam.hpp"
#include <esp32cam.h>

#include <WebServer.h>

#include <WiFi.h>

static const char* WIFI_ssid = "ESP32-CAM";
static const char* WIFI_PASS = "12345678";

esp32cam::Resolution initialResolution;

static esp32cam::Resolution loRes = esp32cam::Resolution::find(320, 240);
static esp32cam::Resolution midRes = esp32cam::Resolution::find (350, 530);
static esp32cam::Resolution hiRes = esp32cam::Resolution::find(800, 600);


WebServer server(80);

void serveJPG ()
{
  //auto frame = esp32cam::capture();
  //esp32cam::Frame frame = esp32cam::capture();
  //while(true)
  
  std::unique_ptr<esp32cam::Frame> frame = esp32cam::capture();
  if (frame == nullptr)
  {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  else
  {
    Serial.println("CAPTURE OK " + String(frame->getWidth()) + " "
                    + String (frame->getHeight()) + " "
                    + String (static_cast<int>(frame->size())));
    server.setContentLength(frame->size());
    server.send(200, "image/jpeg");
    WiFiClient client = server.client();
    frame->writeTo(client);
  }
  
}
// void serveMjpeg() 
// {
//   Serial.println("MJPEG streaming begin");
//   WiFiClient client = server.client();
//   auto startTime = millis();
//   int nFrames = esp32cam::Camera.streamMjpeg(client);
//   auto duration = millis() - startTime;
//   Serial.printf("MJPEG streaming end: %dfrm %0.2ffps\n", nFrames, 1000.0 * nFrames / duration);
// }

void serveMjpeg ()
{
  Serial.println("MJPEG streaming begin");
  WiFiClient client = server.client();

  //Send HTTP headers
  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: multipart/x-mixed-replace; boundary=myboundary\r\n");
  client.print("\r\n");

  while (client.connected())
  {
    //Capture a frame
  std::unique_ptr<esp32cam::Frame> frame = esp32cam::capture();
  if (frame == nullptr)
  {
    Serial.println("CAPTURE FAIL");
    //server.send(503, "", "");
    return;
  }

  //Send boundary delimiter
  client.print("--myboundary\r\n");
  client.print("Content-Type: image/jpeg\r\n");
  client.print("Content-Length: " + String(frame->size()) + "\r\n");
  client.print("\r\n");

  //Send frame data
  frame->writeTo(client);
  delay(50);

  }
  Serial.println("MJPEG streaming end");
  client.stop();
}

void
setup() {
  Serial.begin(115200);
  Serial.println();
  delay(2000);

  
  WiFi.softAP(WIFI_ssid, WIFI_PASS);
  
  Serial.println("WiFi Creted\n Connect to " + String(WIFI_ssid) + ", password: " + String(WIFI_PASS));


  {
    using namespace esp32cam;

    initialResolution = Resolution::find(1024, 768);

    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(initialResolution);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    if (!ok) {
      Serial.println("camera initialize failure");
      delay(5000);
      ESP.restart();
    }
    Serial.println("camera initialize success");
  }

  Serial.println("camera starting");
  Serial.print("http://");
  Serial.print(WiFi.softAPIP());
  Serial.println("/mjpeg");

  //addRequestHandlers();
  server.on("/jpg", serveJPG);
  server.on("/mjpeg", serveMjpeg);
  server.begin();
}

void
loop() {
  server.handleClient();
}
