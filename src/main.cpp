#include <Arduino.h>
#include <WiFi.h>
#include <esp_log.h>
#include <FastLED.h>
#include <Preferences.h>

#include "simple-timer.hpp"
#include "player.hpp"
#include "state-machine.hpp"
#include "device/pdn.hpp"
#include "game/quickdraw.hpp"
#include "id-generator.hpp"
#include "wireless/esp-now-comms.hpp"
#include "wireless/wireless-manager.hpp"
#include "wireless/remote-player-manager.hpp"
#include "game/match-manager.hpp"
#include "wireless/wireless-types.hpp"
#include "wireless/quickdraw-wireless-manager.hpp"
#include "wireless/remote-debug-manager.hpp"

#include "device/display-lights.hpp"
#include "device/grip-lights.hpp"
#include "device/light-manager.hpp"
#include "pdn-rtos.hpp"

// Core game objects
Device* pdn = PDN::GetInstance();
IdGenerator* idGenerator = IdGenerator::GetInstance();
Player* player = new Player();

const char* WIFI_SSID = "NeoCore Networks";
const char* WIFI_PASSWORD = "AlleyCatBountyHunting1";
const char* BASE_URL = "http://alleycat-server.local:3000";

WirelessManager* wirelessManager = new WirelessManager(pdn, WIFI_SSID, WIFI_PASSWORD, BASE_URL);
Quickdraw game = Quickdraw(player, pdn, wirelessManager);

// Remote player management
// RemotePlayerManager remotePlayers;
QuickdrawWirelessManager *quickdrawWirelessManager = QuickdrawWirelessManager::GetInstance();
RemoteDebugManager* remoteDebugManager = RemoteDebugManager::GetInstance();

// Local instance of LightManager for testing
DisplayLights displayLights(13);
GripLights gripLights(6);
LightManager localLightManager(displayLights, gripLights);

// RTOS Initialization
TaskHandle_t deviceTaskHandle = NULL;
TaskHandle_t lightsTaskHandle = NULL;
RingbufHandle_t buf_handle = xRingbufferCreate(1028, RINGBUF_TYPE_NOSPLIT);

void deviceCallback(void *parameter) {
    for (;;) {
        pdn->loop();
        game.loop();
        wirelessManager->loop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void lightsCallback(void *parameter) {
    localLightManager.begin();
    for (;;) {
        size_t item_size;
        void* item = xRingbufferReceive(buf_handle, &item_size, 0);

        if (item != NULL && item_size == sizeof(AnimationConfig)) {
            AnimationConfig config;
            memcpy(&config, item, sizeof(AnimationConfig));

            // start animation
            localLightManager.startAnimation(config);

            // free memory
            vRingbufferReturnItem(buf_handle, (void *)item);
        }

        localLightManager.loop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}


void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);

    player->setUserID(idGenerator->generateId());
    pdn->begin();
    delay(1000);

    ESP_LOGI("PDN", "HW and Game Initialized\n");

    // Initialize wireless manager
    wirelessManager->initialize();
    
    // Set up WiFi for ESP-NOW use - ESP-NOW requires STA mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);  // Don't connect to any AP
    WiFi.channel(6);  // Set a consistent channel for ESP-NOW communication
    
    ESP_LOGI("PDN", "WiFi configured for ESP-NOW");

    remoteDebugManager->Initialize(WIFI_SSID, WIFI_PASSWORD, BASE_URL);

    // Initialize the communications manager
    quickdrawWirelessManager->initialize(player, 1000);

    ESP_LOGI("PDN", "ESP-NOW and Remote Player Service initialized");
    pdn->
    invalidateScreen()->
        drawImage(Quickdraw::getImageForAllegiance(Allegiance::ALLEYCAT, ImageType::LOGO_LEFT))->
        drawImage(Quickdraw::getImageForAllegiance(Allegiance::ALLEYCAT, ImageType::STAMP))->
        render();
    delay(3000);
    game.initialize();

    xTaskCreatePinnedToCore(
        deviceCallback,
        "device task",
        10000,
        NULL,
        3,
        &deviceTaskHandle,
        1
    );

    xTaskCreatePinnedToCore(
        lightsCallback,
        "lights task",
        10000,
        NULL,
        3,
        &lightsTaskHandle,
        1  // Run on CPU 1 to avoid starving IDLE0 watchdog on CPU 0
    );
}

void loop() {
    // pdn->loop();
    // // EspNowManager::GetInstance()->Update();
    // // remotePlayers.Update();
    // game.loop();
    // wirelessManager->loop();
}