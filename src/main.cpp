#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <Wire.h>
#include "fauxmoESP.h"
#include "Adafruit_GFX.h"
#include "Adafruit_LEDBackpack.h"
#include "SPI.h"

Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();
fauxmoESP fauxmo;

#define WIFI_SSID "CMMC_Sinet_2.4G"
#define WIFI_PASS "zxc12345"

#define SERIAL_BAUDRATE 115200

#define ID_MATRIX "matrix lamp"

bool MatrixState = true;
TaskHandle_t MatrixHandle = NULL;
TickType_t DELAY(unsigned long ms)
{
    return ms / portTICK_PERIOD_MS;
}

void wifiSetup()
{
    WiFi.mode(WIFI_STA);

    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }
    Serial.println();

    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void Matrix(void *pvParameters)
{
    for (;;)
    {
        if (MatrixState == true)
        {
            matrix.setRotation(1);
            matrix.setTextSize(1);
            matrix.setTextWrap(false);
            matrix.setTextColor(LED_ON);

            for (int8_t x = 16; x >= -25; x--)
            {
                matrix.clear();
                matrix.setCursor(x, 0);

                matrix.print("On!");
                matrix.writeDisplay();

                vTaskDelay(DELAY(100));
            }

            vTaskDelay(DELAY(120));
        }

        else if (MatrixState == false)
        {
            matrix.setRotation(1);
            matrix.setTextSize(1);
            matrix.setTextWrap(false);
            matrix.setTextColor(LED_ON);

            for (int8_t x = 16; x >= -25; x--)
            {
                matrix.clear();
                matrix.setCursor(x, 0);

                matrix.print("Off!");
                matrix.writeDisplay();

                vTaskDelay(DELAY(100));
            }

            vTaskDelay(DELAY(120));
        }
    }

    vTaskDelete(NULL);
}

void setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    wifiSetup();

    fauxmo.createServer(true);
    fauxmo.setPort(80);

    fauxmo.enable(true);

    matrix.begin(0x70);

    matrix.clear();
    matrix.writeDisplay();

    matrix.setRotation(1);
    matrix.setTextSize(1);
    matrix.setTextWrap(false);
    matrix.setTextColor(LED_ON);

    fauxmo.addDevice(ID_MATRIX);

    xTaskCreate(Matrix, "Matrix Task", 1024, (void *)MatrixState, 1, &MatrixHandle);

    fauxmo.onSetState([](unsigned char device_id, const char *device_name, bool state, unsigned char value) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        if (strcmp(device_name, ID_MATRIX) == 0)
        {
            MatrixState = state ? true : false;
        }
    });
}

void loop()
{
    fauxmo.handle();
    taskYIELD();
}
