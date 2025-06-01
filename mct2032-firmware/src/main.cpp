/**
 * MCT2032 - Mega Cyber Tool 2032
 * ESP32-S3 Main Firmware
 * 
 * Hardware: Waveshare ESP32-S3-LCD-1.47
 * Display: 1.47" Round LCD (172x320) with ST7789 driver
 */

#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include "BLEManager.h"
#include "WiFiScanner.h"
#include "CommandProcessor.h"

// Display configuration
#define TFT_DC 32
#define TFT_CS 14
#define TFT_RST 33
#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO -1
#define TFT_BL 25

// Display dimensions
#define SCREEN_WIDTH 172
#define SCREEN_HEIGHT 320

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[SCREEN_WIDTH * 10];

// Display driver
Arduino_DataBus *bus = new Arduino_ESP32SPI(
    TFT_DC /* DC */, TFT_CS /* CS */,
    TFT_SCK /* SCK */, TFT_MOSI /* MOSI */,
    TFT_MISO /* MISO */, VSPI /* SPI port */
);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus, TFT_RST /* RST */,
    1 /* rotation */, true /* IPS */,
    172 /* width */, 320 /* height */,
    0 /* col offset 1 */, 34 /* row offset 1 */,
    0 /* col offset 2 */, 0 /* row offset 2 */
);

// Component instances
BLEManager bleManager;
WiFiScanner wifiScanner;
CommandProcessor* commandProcessor = nullptr;

// UI elements
lv_obj_t* status_label = nullptr;
lv_obj_t* scan_progress = nullptr;

// LVGL display flush callback
void display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    gfx->startWrite();
    gfx->setAddrWindow(area->x1, area->y1, w, h);
    gfx->writePixels((uint16_t *)color_p, w * h);
    gfx->endWrite();

    lv_disp_flush_ready(disp_drv);
}

// Create UI elements
void create_ui() {
    // Set theme - dark with purple accents
    lv_theme_t *theme = lv_theme_default_init(
        lv_disp_get_default(),
        lv_palette_main(LV_PALETTE_PURPLE),
        lv_palette_main(LV_PALETTE_PURPLE),
        true,
        LV_FONT_DEFAULT
    );
    lv_disp_set_theme(lv_disp_get_default(), theme);

    // Create main screen
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0a0e27), 0);

    // Create circular mask for round display
    lv_obj_t *mask_circle = lv_obj_create(scr);
    lv_obj_set_size(mask_circle, SCREEN_WIDTH, SCREEN_WIDTH);
    lv_obj_align(mask_circle, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_radius(mask_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(mask_circle, lv_color_hex(0x050816), 0);
    lv_obj_set_style_border_width(mask_circle, 3, 0);
    lv_obj_set_style_border_color(mask_circle, lv_palette_main(LV_PALETTE_PURPLE), 0);

    // Create title label
    lv_obj_t *title = lv_label_create(mask_circle);
    lv_label_set_text(title, "MCT2032");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_color(title, lv_palette_main(LV_PALETTE_PURPLE), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);

    // Create subtitle
    lv_obj_t *subtitle = lv_label_create(mask_circle);
    lv_label_set_text(subtitle, "Cyber Tool");
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xcbd5e1), 0);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_16, 0);

    // Create status label
    status_label = lv_label_create(mask_circle);
    lv_label_set_text(status_label, "Initializing...");
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x64748b), 0);

    // Create version info
    lv_obj_t *version = lv_label_create(mask_circle);
    lv_label_set_text(version, "v1.0.0");
    lv_obj_align(version, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_text_color(version, lv_color_hex(0x64748b), 0);
    lv_obj_set_style_text_font(version, &lv_font_montserrat_12, 0);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("MCT2032 Starting...");
    Serial.println("Hardware: Waveshare ESP32-S3-LCD-1.47");

    // Initialize display backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // Initialize display
    gfx->begin();
    gfx->fillScreen(BLACK);
    
    Serial.println("Display initialized");

    // Initialize LVGL
    lv_init();

    // Setup display buffer
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_WIDTH * 10);

    // Initialize display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = display_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Create UI
    create_ui();

    // Initialize components
    bleManager.init();
    wifiScanner.init();
    
    // Create command processor
    commandProcessor = new CommandProcessor(&bleManager, &wifiScanner);
    commandProcessor->init();
    
    // Set BLE command callback
    bleManager.setCommandCallback([](String cmd) {
        if (commandProcessor) {
            commandProcessor->processCommand(cmd);
        }
    });
    
    // Update status
    lv_label_set_text(status_label, "Ready - Waiting for connection");

    Serial.println("MCT2032 Ready!");
}

void loop() {
    // Handle LVGL
    lv_timer_handler();
    
    // Check BLE connection
    bleManager.checkConnection();
    
    // Check WiFi scan status
    if (wifiScanner.isScanning()) {
        static unsigned long lastUpdate = 0;
        if (millis() - lastUpdate > 100) {
            lastUpdate = millis();
            lv_label_set_text(status_label, "Scanning WiFi...");
        }
    } else if (wifiScanner.getNetworkCount() > 0) {
        // Send results if scan completed
        static bool resultsSent = false;
        if (!resultsSent) {
            DynamicJsonDocument doc(4096);
            wifiScanner.toJSON(doc);
            bleManager.sendResponse(CMD_SCAN_WIFI, STATUS_SUCCESS, doc);
            
            char buf[64];
            snprintf(buf, sizeof(buf), "Found %d networks", wifiScanner.getNetworkCount());
            lv_label_set_text(status_label, buf);
            
            resultsSent = true;
            
            // Reset after delay
            static unsigned long resetTime = millis() + 3000;
            if (millis() > resetTime) {
                resultsSent = false;
                if (bleManager.isConnected()) {
                    lv_label_set_text(status_label, "Connected - Ready");
                }
            }
        }
    }
    
    // Update connection status
    static bool wasConnected = false;
    bool isConnected = bleManager.isConnected();
    if (isConnected != wasConnected) {
        wasConnected = isConnected;
        if (isConnected) {
            lv_label_set_text(status_label, "Connected - Ready");
        } else {
            lv_label_set_text(status_label, "Disconnected");
        }
    }
    
    delay(5);
}