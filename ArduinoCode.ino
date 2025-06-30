#include "esp_camera.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "esp_http_server.h"


#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define LED_PIN 4

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22


const char* ssid = "ipd";
const char* password = "rrrrrrrr";


const char* serverUrl = "http://192.168.xx.xx:3000/ocr";


bool cameraInitialized = false;
bool streamingActive = false;


httpd_handle_t stream_httpd = NULL;


unsigned long lastServerSendTime = 0;
const unsigned long serverSendInterval = 7000;


void send_image_to_server(camera_fb_t *fb) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "image/jpeg");

        int httpResponseCode = http.POST(fb->buf, fb->len);
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Server Response: " + response);
        } else {
            Serial.print("Error in sending image. HTTP Response code: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi is not connected, cannot send image");
    }
}


esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char part_buf[64];

    res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
    if (res != ESP_OK) {
        return res;
    }

    streamingActive = true;

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            if (fb->format == PIXFORMAT_JPEG) {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;

                size_t hlen = snprintf(part_buf, 64, "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", _jpg_buf_len);
                res = httpd_resp_send_chunk(req, part_buf, hlen);
                if (res == ESP_OK) {
                    res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
                }
                if (res == ESP_OK) {
                    res = httpd_resp_send_chunk(req, "\r\n--frame\r\n", 13);
                }

                // Check if 7 seconds have passed to send image to the server
                if (millis() - lastServerSendTime >= serverSendInterval) {
                    Serial.println("Sending image to server during streaming...");
                    send_image_to_server(fb);  // Send the image to the server
                    lastServerSendTime = millis(); // Update last send time
                }
            }
        }

        esp_camera_fb_return(fb);

        if (res != ESP_OK) {
            break;
        }
    }

    streamingActive = false; // Streaming stopped
    return res;
}


void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();


    httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}


void capture_and_send_image() {
    if (!cameraInitialized) {
        Serial.println("Camera not initialized");
        return;
    }

    if (!streamingActive) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Failed to capture image");
            return;
        }

        Serial.println("Sending image to server in normal mode...");
        send_image_to_server(fb);  // Send the image to the server

        esp_camera_fb_return(fb);
    }
}


void captureTask(void *pvParameters) {
    while (true) {
        capture_and_send_image();
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay for 5 seconds
    }
}

void setup() {
    Serial.begin(115200);


    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN, HIGH);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  
  //config.pixel_format = PIXFORMAT_RGB565; 
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  
  
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {      
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

        
    if (esp_camera_init(&config) == ESP_OK) {
        Serial.println("Camera initialized successfully");
        cameraInitialized = true;
    } else {
        Serial.println("Camera initialization failed");
    }

    
    startCameraServer();
    Serial.println(WiFi.localIP());
    Serial.println("Camera Stream Ready: Connect to http://<ESP32_IP>/stream to see preview");

    
    xTaskCreate(captureTask, "CaptureTask", 2048, NULL, 1, NULL);
}

void loop() {
    
}
