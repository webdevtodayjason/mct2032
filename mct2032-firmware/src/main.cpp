/**
 * MCT2032 - Mini Cyber Tool 2032
 * ESP32-S3 Main Firmware
 * 
 * Hardware: Waveshare ESP32-S3-LCD-1.47
 * Display: 1.47" Round LCD (172x320) with ST7789 driver
 */

#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <Adafruit_NeoPixel.h>
#include "BLEManager.h"
#include "WiFiScanner.h"
#include "CommandProcessor.h"
#include "PacketMonitor.h"

// Declare fonts - commented out as they're not properly linked
// LV_FONT_DECLARE(lv_font_montserrat_12)
// LV_FONT_DECLARE(lv_font_montserrat_16)
// LV_FONT_DECLARE(lv_font_montserrat_24)

// Display configuration for Waveshare ESP32-S3-LCD-1.47
#define TFT_DC 41
#define TFT_CS 42
#define TFT_RST 39
#define TFT_SCK 40
#define TFT_MOSI 45
#define TFT_MISO -1
#define TFT_BL 48

// RGB LED configuration
#define RGB_LED_PIN 38
#define RGB_LED_COUNT 1

// Display dimensions for Waveshare ESP32-S3-LCD-1.47
#define SCREEN_WIDTH 172
#define SCREEN_HEIGHT 320
#define SCREEN_RADIUS 86  // Half of width for circular display

// LVGL display buffer - larger buffer for smoother rendering
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[SCREEN_WIDTH * 20];  // Increased buffer size

// Display driver - use HSPI for ESP32-S3
Arduino_GFX *gfx = nullptr;

// Component instances
BLEManager bleManager;
WiFiScanner wifiScanner;
PacketMonitor packetMonitor;
CommandProcessor* commandProcessor = nullptr;

// RGB LED instance
Adafruit_NeoPixel rgbLED(RGB_LED_COUNT, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

// UI elements
lv_obj_t* status_label = nullptr;
lv_obj_t* scan_progress = nullptr;
lv_obj_t* mode_label = nullptr;
lv_obj_t* stats_label = nullptr;
lv_obj_t* network_count_label = nullptr;
lv_obj_t* connection_icon = nullptr;

// LVGL display flush callback
void display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    // Use the swapped color format for ST7789
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);

    lv_disp_flush_ready(disp_drv);
}

// Create UI elements
void create_ui() {
    // Create main screen with deep dark background
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0a0e27), 0);

    // Create circular mask for round display with gradient border effect
    lv_obj_t *mask_circle = lv_obj_create(scr);
    lv_obj_set_size(mask_circle, SCREEN_WIDTH - 2, SCREEN_WIDTH - 2);  // 170x170 circle
    lv_obj_align(mask_circle, LV_ALIGN_TOP_MID, 0, (SCREEN_HEIGHT - SCREEN_WIDTH) / 2);
    lv_obj_set_style_radius(mask_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(mask_circle, lv_color_hex(0x0f0f23), 0);
    lv_obj_set_style_border_width(mask_circle, 3, 0);
    lv_obj_set_style_border_color(mask_circle, lv_color_hex(0x8b5cf6), 0);
    lv_obj_set_style_shadow_color(mask_circle, lv_color_hex(0x7c3aed), 0);
    lv_obj_set_style_shadow_width(mask_circle, 8, 0);
    lv_obj_set_style_shadow_spread(mask_circle, 2, 0);
    lv_obj_clear_flag(mask_circle, LV_OBJ_FLAG_SCROLLABLE);

    // Top section - Title with glitch effect styling
    lv_obj_t *title_container = lv_obj_create(mask_circle);
    lv_obj_set_size(title_container, 140, 35);
    lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(title_container, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_border_width(title_container, 1, 0);
    lv_obj_set_style_border_color(title_container, lv_color_hex(0x7c3aed), 0);
    lv_obj_set_style_radius(title_container, 5, 0);
    lv_obj_clear_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(title_container);
    lv_label_set_text(title, "MCT-2032");
    lv_obj_center(title);
    lv_obj_set_style_text_color(title, lv_color_hex(0xe0aaff), 0);
    // lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);

    // Connection status indicator (top right)
    connection_icon = lv_obj_create(mask_circle);
    lv_obj_set_size(connection_icon, 8, 8);
    lv_obj_align(connection_icon, LV_ALIGN_TOP_RIGHT, -15, 15);
    lv_obj_set_style_radius(connection_icon, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(connection_icon, lv_color_hex(0xff0000), 0);  // Red = disconnected
    lv_obj_set_style_border_width(connection_icon, 0, 0);

    // Mode indicator with cyber styling
    mode_label = lv_label_create(mask_circle);
    lv_label_set_text(mode_label, "[ IDLE ]");
    lv_obj_align(mode_label, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_text_color(mode_label, lv_color_hex(0x00ff00), 0);  // Green text
    // lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_12, 0);

    // Center section - Main stats display
    lv_obj_t *stats_container = lv_obj_create(mask_circle);
    lv_obj_set_size(stats_container, 130, 50);
    lv_obj_align(stats_container, LV_ALIGN_CENTER, 0, -5);
    lv_obj_set_style_bg_opa(stats_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(stats_container, 1, 0);
    lv_obj_set_style_border_color(stats_container, lv_color_hex(0x3f3f74), 0);
    lv_obj_set_style_border_opa(stats_container, LV_OPA_50, 0);
    lv_obj_clear_flag(stats_container, LV_OBJ_FLAG_SCROLLABLE);

    stats_label = lv_label_create(stats_container);
    lv_label_set_text(stats_label, "READY");
    lv_obj_center(stats_label);
    lv_obj_set_style_text_color(stats_label, lv_color_hex(0x10ee10), 0);  // Matrix green
    // lv_obj_set_style_text_font(stats_label, &lv_font_montserrat_14, 0);

    // Network count display (when scanning)
    network_count_label = lv_label_create(mask_circle);
    lv_label_set_text(network_count_label, "");
    lv_obj_align(network_count_label, LV_ALIGN_CENTER, 0, 25);
    lv_obj_set_style_text_color(network_count_label, lv_color_hex(0x64748b), 0);
    // lv_obj_set_style_text_font(network_count_label, &lv_font_montserrat_12, 0);

    // Bottom section - Status messages with terminal style
    lv_obj_t *terminal_container = lv_obj_create(mask_circle);
    lv_obj_set_size(terminal_container, 140, 25);
    lv_obj_align(terminal_container, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(terminal_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(terminal_container, 1, 0);
    lv_obj_set_style_border_color(terminal_container, lv_color_hex(0x2d2d52), 0);
    lv_obj_set_style_radius(terminal_container, 3, 0);
    lv_obj_clear_flag(terminal_container, LV_OBJ_FLAG_SCROLLABLE);

    status_label = lv_label_create(terminal_container);
    lv_label_set_text(status_label, "> INIT...");
    lv_obj_align(status_label, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x00ff41), 0);  // Terminal green
    // Use smaller default font instead of montserrat_10
    
    // ASCII art decoration (optional fun element)
    lv_obj_t *deco = lv_label_create(mask_circle);
    lv_label_set_text(deco, "* * *");
    lv_obj_align(deco, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_text_color(deco, lv_color_hex(0x3f3f74), 0);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("MCT2032 Starting...");
    Serial.println("Hardware: Waveshare ESP32-S3-LCD-1.47");

    // Initialize RGB LED first (to show we're alive)
    rgbLED.begin();
    rgbLED.setBrightness(50);  // Not too bright
    rgbLED.setPixelColor(0, rgbLED.Color(255, 0, 0));  // Red = booting
    rgbLED.show();

    // Initialize display backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // Initialize display with correct configuration
    Arduino_DataBus *bus = new Arduino_ESP32SPI(
        TFT_DC,    // DC pin (41)
        TFT_CS,    // CS pin (42)
        TFT_SCK,   // SCK pin (40)
        TFT_MOSI,  // MOSI pin (45)
        TFT_MISO   // MISO (-1, not used)
    );
    
    // ST7789 display configuration for Waveshare ESP32-S3-LCD-1.47
    // The physical display is 172x320 in portrait orientation
    gfx = new Arduino_ST7789(
        bus,
        TFT_RST,   // Reset pin (39)
        0,         // Rotation 0 = portrait (172 wide x 320 tall)
        true,      // IPS panel
        172,       // Width
        320,       // Height
        34,        // Col offset 1
        0,         // Row offset 1
        34,        // Col offset 2
        0          // Row offset 2
    );

    // Initialize display
    gfx->begin();
    gfx->fillScreen(BLACK);
    
    Serial.println("Display initialized");
    
    // Change LED to orange = display initialized
    rgbLED.setPixelColor(0, rgbLED.Color(255, 165, 0));
    rgbLED.show();

    // Initialize LVGL
    lv_init();

    // Setup display buffer
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_WIDTH * 20);

    // Initialize display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = display_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.antialiasing = 0;  // Disable anti-aliasing (can cause color fringing on some displays)
    disp_drv.dpi = 130;  // Set DPI for proper font rendering
    lv_disp_drv_register(&disp_drv);

    // Create UI
    create_ui();

    // Initialize components
    bleManager.init();
    wifiScanner.init();
    // Note: packetMonitor.init() only sets up the instance, doesn't enable promiscuous mode
    packetMonitor.init();
    
    // Create command processor
    commandProcessor = new CommandProcessor(&bleManager, &wifiScanner, &packetMonitor);
    commandProcessor->init();
    
    // Set BLE command callback
    bleManager.setCommandCallback([](String cmd) {
        if (commandProcessor) {
            commandProcessor->processCommand(cmd);
        }
    });
    
    // Update status
    lv_label_set_text(status_label, "> WAITING...");
    lv_label_set_text(stats_label, "NO LINK");

    // Change LED to purple = ready
    rgbLED.setPixelColor(0, rgbLED.Color(128, 0, 128));
    rgbLED.show();
    
    Serial.println("MCT2032 Ready!");
}

void loop() {
    // Handle LVGL
    lv_timer_handler();
    
    // Note: Command processor mode management would go here if we had getter/setter methods
    
    // Update RGB LED based on status
    static unsigned long lastLedUpdate = 0;
    static uint8_t breathValue = 0;
    static bool breathDirection = true;
    
    if (millis() - lastLedUpdate > 20) {  // Update every 20ms
        lastLedUpdate = millis();
        
        // Breathing effect
        if (breathDirection) {
            breathValue += 5;
            if (breathValue >= 100) breathDirection = false;
        } else {
            breathValue -= 5;
            if (breathValue <= 20) breathDirection = true;
        }
        
        // Set color based on state
        if (wifiScanner.isScanning()) {
            // Blue = scanning
            rgbLED.setPixelColor(0, rgbLED.Color(0, 0, breathValue));
        } else if (bleManager.isConnected()) {
            // Green = connected
            rgbLED.setPixelColor(0, rgbLED.Color(0, breathValue, 0));
        } else {
            // Purple = idle/ready
            rgbLED.setPixelColor(0, rgbLED.Color(breathValue/2, 0, breathValue/2));
        }
        rgbLED.show();
    }
    
    // Check BLE connection and update UI
    bleManager.checkConnection();
    
    // Update connection indicator
    static bool lastConnectedState = false;
    if (bleManager.isConnected() != lastConnectedState) {
        lastConnectedState = bleManager.isConnected();
        if (lastConnectedState) {
            lv_obj_set_style_bg_color(connection_icon, lv_color_hex(0x00ff00), 0);  // Green = connected
            lv_label_set_text(status_label, "> LINKED");
            lv_label_set_text(stats_label, "ONLINE");
        } else {
            lv_obj_set_style_bg_color(connection_icon, lv_color_hex(0xff0000), 0);  // Red = disconnected
            lv_label_set_text(status_label, "> NO LINK");
            lv_label_set_text(stats_label, "OFFLINE");
        }
    }
    
    // Update mode display
    static uint8_t lastMode = MODE_IDLE;
    if (commandProcessor && commandProcessor->getCurrentMode() != lastMode) {
        lastMode = commandProcessor->getCurrentMode();
        switch (lastMode) {
            case MODE_SCANNING:
                lv_label_set_text(mode_label, "[ SCANNING ]");
                lv_obj_set_style_text_color(mode_label, lv_color_hex(0x00bfff), 0);  // Cyan
                break;
            case MODE_MONITORING:
                lv_label_set_text(mode_label, "[ MONITOR ]");
                lv_obj_set_style_text_color(mode_label, lv_color_hex(0xff4500), 0);  // Orange
                break;
            case MODE_ATTACKING:
                lv_label_set_text(mode_label, "[ ATTACK ]");
                lv_obj_set_style_text_color(mode_label, lv_color_hex(0xff0000), 0);  // Red
                break;
            case MODE_BEACON_SPAM:
                lv_label_set_text(mode_label, "[ BEACON ]");
                lv_obj_set_style_text_color(mode_label, lv_color_hex(0xff1493), 0);  // Pink
                break;
            case MODE_PCAP_CAPTURE:
                lv_label_set_text(mode_label, "[ CAPTURE ]");
                lv_obj_set_style_text_color(mode_label, lv_color_hex(0x9370db), 0);  // Purple
                break;
            default:
                lv_label_set_text(mode_label, "[ IDLE ]");
                lv_obj_set_style_text_color(mode_label, lv_color_hex(0x00ff00), 0);  // Green
                break;
        }
    }
    
    // Check WiFi scan status - improved based on Marauder pattern
    static bool scanRequested = false;
    static bool scanInProgress = false;
    static unsigned long scanStartTime = 0;
    
    // Check if we have a pending scan request
    if (commandProcessor && commandProcessor->getCurrentMode() == MODE_SCANNING && !scanInProgress) {
        scanRequested = true;
        scanInProgress = true;
        scanStartTime = millis();
        Serial.println("=== WiFi SCAN STARTED ===");
        Serial.printf("Mode: %d, Time: %lu\n", commandProcessor->getCurrentMode(), millis());
    }
    
    if (scanInProgress) {
        // Update display during scan
        static unsigned long lastUpdate = 0;
        static int animFrame = 0;
        if (millis() - lastUpdate > 100) {
            lastUpdate = millis();
            
            // Animated scanning indicator
            const char* scanAnim[] = {
                "> SCAN [|]",
                "> SCAN [/]", 
                "> SCAN [-]",
                "> SCAN [\\]"
            };
            lv_label_set_text(status_label, scanAnim[animFrame % 4]);
            animFrame++;
            
            // Show elapsed time
            char buf[32];
            snprintf(buf, sizeof(buf), "%d ms", (int)(millis() - scanStartTime));
            lv_label_set_text(stats_label, buf);
        }
        
        // Check if scan is complete
        if (!wifiScanner.isScanning()) {
            Serial.printf("Scan complete. Found %d networks\n", wifiScanner.getNetworkCount());
            
            // Send results
            Serial.println("=== SENDING WIFI SCAN RESULTS ===");
            
            // Use larger document size for WiFi results
            DynamicJsonDocument doc(8192);
            if (wifiScanner.getNetworkCount() > 0) {
                wifiScanner.toJSON(doc);
                
                // Update display with results
                char statusBuf[32];
                snprintf(statusBuf, sizeof(statusBuf), "> FOUND: %d", wifiScanner.getNetworkCount());
                lv_label_set_text(status_label, statusBuf);
                
                char statsBuf[32];
                snprintf(statsBuf, sizeof(statsBuf), "%d APs", wifiScanner.getNetworkCount());
                lv_label_set_text(stats_label, statsBuf);
                
                // Show network count
                char countBuf[64];
                snprintf(countBuf, sizeof(countBuf), "%d networks detected", wifiScanner.getNetworkCount());
                lv_label_set_text(network_count_label, countBuf);
                
                // Check JSON size
                size_t jsonSize = measureJson(doc);
                Serial.printf("Sending %d networks, JSON size: %d bytes\n", 
                              wifiScanner.getNetworkCount(), jsonSize);
                
                if (jsonSize > 2000) {
                    Serial.println("WARNING: Response may be too large for BLE!");
                }
            } else {
                JsonArray emptyArray = doc.createNestedArray("networks");
                lv_label_set_text(status_label, "> NO NETS");
                lv_label_set_text(stats_label, "0 APs");
                lv_label_set_text(network_count_label, "No networks found");
                Serial.println("Sending empty network list");
            }
            
            // Send the scan results using chunked method for large data
            if (!bleManager.sendChunkedResponse(CMD_SCAN_WIFI, STATUS_SUCCESS, doc)) {
                Serial.println("ERROR: Failed to send WiFi scan results!");
            } else {
                Serial.println("WiFi scan results sent successfully");
            }
            
            // Reset state
            scanInProgress = false;
            scanRequested = false;
            if (commandProcessor) {
                commandProcessor->setMode(MODE_IDLE);
            }
        }
        
        // Timeout check (10 seconds)
        if (millis() - scanStartTime > 10000) {
            Serial.println("WiFi scan timeout!");
            bleManager.sendError(CMD_SCAN_WIFI, "Scan timeout");
            scanInProgress = false;
            scanRequested = false;
            if (commandProcessor) {
                commandProcessor->setMode(MODE_IDLE);
            }
            lv_label_set_text(status_label, "Scan timeout");
        }
    }
    
    // Reset status after displaying results
    static unsigned long statusResetTime = 0;
    static bool statusDisplayed = false;
    
    // Set timer when scan completes
    if (!scanInProgress && scanRequested && !statusDisplayed) {
        statusResetTime = millis() + 5000;  // Show status for 5 seconds
        statusDisplayed = true;
    }
    
    // Reset display after timeout
    if (statusResetTime > 0 && millis() > statusResetTime) {
        // Clear network count
        lv_label_set_text(network_count_label, "");
        
        // Reset to idle display
        if (bleManager.isConnected()) {
            lv_label_set_text(status_label, "> READY");
            lv_label_set_text(stats_label, "ONLINE");
        } else {
            lv_label_set_text(status_label, "> NO LINK");
            lv_label_set_text(stats_label, "OFFLINE");
        }
        
        statusResetTime = 0;
        statusDisplayed = false;
        scanRequested = false;
    }
    
    // Update packet monitor stats if monitoring
    if (commandProcessor && commandProcessor->getCurrentMode() == MODE_MONITORING) {
        static unsigned long lastMonitorUpdate = 0;
        if (millis() - lastMonitorUpdate > 500) {  // Update every 500ms
            lastMonitorUpdate = millis();
            
            char buf[64];
            snprintf(buf, sizeof(buf), "PKT: %lu", packetMonitor.getPacketsTotal());
            lv_label_set_text(stats_label, buf);
            
            snprintf(buf, sizeof(buf), "%lu pkt/s", packetMonitor.getPacketsPerSec());
            lv_label_set_text(network_count_label, buf);
        }
    }
    
    delay(5);
}