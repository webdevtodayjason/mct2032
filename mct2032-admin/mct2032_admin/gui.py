"""
Dark-themed GUI for MCT2032 Admin Console
Using customtkinter with shadcn/ui inspired styling
"""

import tkinter as tk
from tkinter import ttk
import customtkinter as ctk
import asyncio
import threading
from typing import Optional, Dict, Any
from queue import Queue, Empty
import logging
from datetime import datetime

from .protocol import Protocol, ResponseStatus, NetworkInfo, DeviceStatus, PacketStats
from .ble_controller import BLEController


logger = logging.getLogger(__name__)

# Theme configuration
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("dark-blue")


class CyberToolGUI:
    """Main GUI application for MCT2032 Admin Console"""
    
    def __init__(self):
        self.root = ctk.CTk()
        self.root.title("MCT2032 - Mega Cyber Tool 2032")
        self.root.geometry("1200x800")
        
        # Dark theme colors (matching shadcn/ui)
        self.colors = {
            "bg_primary": "#0a0e27",
            "bg_secondary": "#050816",
            "bg_tertiary": "#0f172a",
            "fg_primary": "#f8fafc",
            "fg_secondary": "#cbd5e1",
            "fg_muted": "#64748b",
            "purple": "#8b5cf6",
            "purple_dark": "#7c3aed",
            "border": "#1e293b",
            "success": "#10b981",
            "warning": "#f59e0b",
            "error": "#ef4444"
        }
        
        # Configure root window
        self.root.configure(bg=self.colors["bg_primary"])
        
        # Queue for BLE updates
        self.update_queue = Queue()
        self.ble_controller: Optional[BLEController] = None
        self.async_loop: Optional[asyncio.AbstractEventLoop] = None
        self.async_thread: Optional[threading.Thread] = None
        
        # Data storage
        self.wifi_networks = []
        self.ble_devices = []
        self.packet_stats = None
        self.device_status = None
        
        # Setup UI
        self._setup_styles()
        self._create_widgets()
        self._setup_async_handler()
        
        # Start UI update loop
        self.root.after(100, self._process_updates)
    
    def _setup_styles(self):
        """Configure custom styles"""
        style = ttk.Style()
        style.theme_use('clam')
        
        # Configure dark theme for ttk widgets
        style.configure(
            "Dark.Treeview",
            background=self.colors["bg_secondary"],
            foreground=self.colors["fg_primary"],
            fieldbackground=self.colors["bg_secondary"],
            bordercolor=self.colors["border"],
            darkcolor=self.colors["bg_primary"],
            lightcolor=self.colors["bg_tertiary"],
            insertcolor=self.colors["purple"]
        )
        
        style.configure(
            "Dark.Treeview.Heading",
            background=self.colors["bg_tertiary"],
            foreground=self.colors["fg_primary"],
            bordercolor=self.colors["border"]
        )
        
        style.map(
            "Dark.Treeview",
            background=[("selected", self.colors["purple_dark"])],
            foreground=[("selected", self.colors["fg_primary"])]
        )
    
    def _create_widgets(self):
        """Create all GUI widgets"""
        # Top frame - Connection status and controls
        self._create_top_frame()
        
        # Main content - Tabbed interface
        self._create_main_content()
        
        # Bottom frame - Status bar
        self._create_status_bar()
    
    def _create_top_frame(self):
        """Create top frame with connection controls"""
        top_frame = ctk.CTkFrame(
            self.root,
            fg_color=self.colors["bg_secondary"],
            corner_radius=0
        )
        top_frame.pack(fill="x", padx=0, pady=0)
        
        # Logo/Title
        title_label = ctk.CTkLabel(
            top_frame,
            text="MCT2032 ADMIN CONSOLE",
            font=("SF Pro Display", 24, "bold"),
            text_color=self.colors["purple"]
        )
        title_label.pack(side="left", padx=20, pady=15)
        
        # Connection frame
        conn_frame = ctk.CTkFrame(
            top_frame,
            fg_color="transparent"
        )
        conn_frame.pack(side="right", padx=20, pady=15)
        
        # Connection status indicator
        self.conn_status_label = ctk.CTkLabel(
            conn_frame,
            text="● Disconnected",
            font=("SF Pro Display", 14),
            text_color=self.colors["error"]
        )
        self.conn_status_label.pack(side="left", padx=10)
        
        # Connect button with gradient effect
        self.connect_btn = ctk.CTkButton(
            conn_frame,
            text="Connect",
            command=self._on_connect_click,
            fg_color=self.colors["purple"],
            hover_color=self.colors["purple_dark"],
            border_width=2,
            border_color=self.colors["purple"],
            corner_radius=8,
            font=("SF Pro Display", 14, "bold"),
            width=120,
            height=40
        )
        self.connect_btn.pack(side="left", padx=5)
    
    def _create_main_content(self):
        """Create main tabbed interface"""
        # Tab view with custom styling
        self.tabview = ctk.CTkTabview(
            self.root,
            fg_color=self.colors["bg_primary"],
            segmented_button_fg_color=self.colors["bg_secondary"],
            segmented_button_selected_color=self.colors["purple"],
            segmented_button_unselected_color=self.colors["bg_tertiary"],
            segmented_button_selected_hover_color=self.colors["purple_dark"],
            text_color=self.colors["fg_secondary"],
            corner_radius=10
        )
        self.tabview.pack(fill="both", expand=True, padx=20, pady=(0, 10))
        
        # Create tabs
        self.wifi_tab = self.tabview.add("WiFi Scanner")
        self.ble_tab = self.tabview.add("Bluetooth")
        self.packet_tab = self.tabview.add("Packet Monitor")
        self.console_tab = self.tabview.add("Console")
        
        # Setup each tab
        self._setup_wifi_tab()
        self._setup_ble_tab()
        self._setup_packet_tab()
        self._setup_console_tab()
    
    def _setup_wifi_tab(self):
        """Setup WiFi scanner tab"""
        # Control frame
        control_frame = ctk.CTkFrame(
            self.wifi_tab,
            fg_color=self.colors["bg_secondary"],
            corner_radius=10
        )
        control_frame.pack(fill="x", padx=10, pady=10)
        
        # Scan button
        self.wifi_scan_btn = ctk.CTkButton(
            control_frame,
            text="Scan Networks",
            command=self._on_wifi_scan_click,
            fg_color=self.colors["purple"],
            hover_color=self.colors["purple_dark"],
            corner_radius=8,
            font=("SF Pro Display", 14),
            width=150,
            height=40
        )
        self.wifi_scan_btn.pack(side="left", padx=10, pady=10)
        
        # Stop scan button
        self.wifi_stop_btn = ctk.CTkButton(
            control_frame,
            text="Stop Scan",
            command=self._on_wifi_stop_click,
            fg_color=self.colors["error"],
            hover_color="#991b1b",
            corner_radius=8,
            font=("SF Pro Display", 14),
            width=100,
            height=40,
            state="disabled"
        )
        self.wifi_stop_btn.pack(side="left", padx=0, pady=10)
        
        # Channel selector
        channel_label = ctk.CTkLabel(
            control_frame,
            text="Channel:",
            font=("SF Pro Display", 14),
            text_color=self.colors["fg_secondary"]
        )
        channel_label.pack(side="left", padx=(20, 5))
        
        self.channel_var = tk.StringVar(value="All")
        channel_menu = ctk.CTkOptionMenu(
            control_frame,
            values=["All"] + [str(i) for i in range(1, 15)],
            variable=self.channel_var,
            fg_color=self.colors["bg_tertiary"],
            button_color=self.colors["purple"],
            button_hover_color=self.colors["purple_dark"],
            dropdown_fg_color=self.colors["bg_secondary"],
            dropdown_hover_color=self.colors["purple_dark"],
            corner_radius=8,
            width=100
        )
        channel_menu.pack(side="left", padx=5)
        
        # Results count
        self.wifi_count_label = ctk.CTkLabel(
            control_frame,
            text="Networks: 0",
            font=("SF Pro Display", 14),
            text_color=self.colors["fg_muted"]
        )
        self.wifi_count_label.pack(side="right", padx=10)
        
        # Results treeview
        tree_frame = ctk.CTkFrame(
            self.wifi_tab,
            fg_color=self.colors["bg_secondary"],
            corner_radius=10
        )
        tree_frame.pack(fill="both", expand=True, padx=10, pady=(0, 10))
        
        # Create Treeview with scrollbar
        tree_scroll = ttk.Scrollbar(tree_frame)
        tree_scroll.pack(side="right", fill="y")
        
        self.wifi_tree = ttk.Treeview(
            tree_frame,
            yscrollcommand=tree_scroll.set,
            style="Dark.Treeview",
            show="tree headings",
            height=15
        )
        tree_scroll.config(command=self.wifi_tree.yview)
        
        # Configure columns
        self.wifi_tree["columns"] = ("SSID", "BSSID", "Channel", "RSSI", "Security")
        self.wifi_tree.column("#0", width=0, stretch=False)
        self.wifi_tree.column("SSID", width=250)
        self.wifi_tree.column("BSSID", width=150)
        self.wifi_tree.column("Channel", width=80)
        self.wifi_tree.column("RSSI", width=80)
        self.wifi_tree.column("Security", width=150)
        
        # Configure headings
        for col in self.wifi_tree["columns"]:
            self.wifi_tree.heading(col, text=col)
        
        self.wifi_tree.pack(fill="both", expand=True, padx=5, pady=5)
    
    def _setup_ble_tab(self):
        """Setup Bluetooth scanner tab"""
        # Similar structure to WiFi tab
        # Control frame
        control_frame = ctk.CTkFrame(
            self.ble_tab,
            fg_color=self.colors["bg_secondary"],
            corner_radius=10
        )
        control_frame.pack(fill="x", padx=10, pady=10)
        
        # Scan button
        self.ble_scan_btn = ctk.CTkButton(
            control_frame,
            text="Scan Devices",
            command=self._on_ble_scan_click,
            fg_color=self.colors["purple"],
            hover_color=self.colors["purple_dark"],
            corner_radius=8,
            font=("SF Pro Display", 14),
            width=150,
            height=40
        )
        self.ble_scan_btn.pack(side="left", padx=10, pady=10)
        
        # Device count
        self.ble_count_label = ctk.CTkLabel(
            control_frame,
            text="Devices: 0",
            font=("SF Pro Display", 14),
            text_color=self.colors["fg_muted"]
        )
        self.ble_count_label.pack(side="right", padx=10)
    
    def _setup_packet_tab(self):
        """Setup packet monitor tab"""
        # Stats display frame
        stats_frame = ctk.CTkFrame(
            self.packet_tab,
            fg_color=self.colors["bg_secondary"],
            corner_radius=10
        )
        stats_frame.pack(fill="x", padx=10, pady=10)
        
        # Create stat cards
        self.stat_labels = {}
        stats = [
            ("Total Packets", "0"),
            ("Packets/sec", "0"),
            ("Beacons", "0"),
            ("Probes", "0"),
            ("Data", "0"),
            ("Management", "0")
        ]
        
        for i, (label, value) in enumerate(stats):
            card = self._create_stat_card(stats_frame, label, value)
            card.grid(row=0, column=i, padx=5, pady=5, sticky="ew")
            stats_frame.columnconfigure(i, weight=1)
    
    def _create_stat_card(self, parent, label, value):
        """Create a statistics card widget"""
        card = ctk.CTkFrame(
            parent,
            fg_color=self.colors["bg_tertiary"],
            corner_radius=8,
            border_width=2,
            border_color=self.colors["purple"]
        )
        
        label_widget = ctk.CTkLabel(
            card,
            text=label,
            font=("SF Pro Display", 12),
            text_color=self.colors["fg_muted"]
        )
        label_widget.pack(pady=(10, 5))
        
        value_widget = ctk.CTkLabel(
            card,
            text=value,
            font=("SF Pro Display", 24, "bold"),
            text_color=self.colors["purple"]
        )
        value_widget.pack(pady=(0, 10))
        
        self.stat_labels[label] = value_widget
        return card
    
    def _setup_console_tab(self):
        """Setup console/log tab"""
        # Console text widget
        self.console_text = ctk.CTkTextbox(
            self.console_tab,
            fg_color=self.colors["bg_secondary"],
            text_color=self.colors["fg_primary"],
            font=("SF Mono", 12),
            corner_radius=10
        )
        self.console_text.pack(fill="both", expand=True, padx=10, pady=10)
        
        # Welcome message
        self.log_message("MCT2032 Admin Console v1.0.0", "info")
        self.log_message("Ready to connect to device...", "info")
    
    def _create_status_bar(self):
        """Create bottom status bar"""
        status_frame = ctk.CTkFrame(
            self.root,
            fg_color=self.colors["bg_secondary"],
            corner_radius=0,
            height=40
        )
        status_frame.pack(fill="x", side="bottom")
        status_frame.pack_propagate(False)
        
        # Status text
        self.status_label = ctk.CTkLabel(
            status_frame,
            text="Ready",
            font=("SF Pro Display", 12),
            text_color=self.colors["fg_muted"]
        )
        self.status_label.pack(side="left", padx=20)
        
        # Device info
        self.device_info_label = ctk.CTkLabel(
            status_frame,
            text="",
            font=("SF Pro Display", 12),
            text_color=self.colors["fg_muted"]
        )
        self.device_info_label.pack(side="right", padx=20)
    
    def _setup_async_handler(self):
        """Setup async event loop in separate thread"""
        self.async_loop = asyncio.new_event_loop()
        self.async_thread = threading.Thread(
            target=self._run_async_loop,
            daemon=True
        )
        self.async_thread.start()
        
        # Create BLE controller
        self.ble_controller = BLEController(self.update_queue)
    
    def _run_async_loop(self):
        """Run async event loop"""
        asyncio.set_event_loop(self.async_loop)
        self.async_loop.run_forever()
    
    def _process_updates(self):
        """Process updates from BLE controller"""
        try:
            while True:
                update_type, data = self.update_queue.get_nowait()
                
                if update_type == "data":
                    self._handle_data_update(data)
                elif update_type == "status":
                    self._handle_status_update(data)
                    
        except Empty:
            pass
        
        # Schedule next update
        self.root.after(100, self._process_updates)
    
    def _handle_data_update(self, data: Dict[str, Any]):
        """Handle data updates from device"""
        if "networks" in data.get("data", {}):
            # WiFi scan results
            networks = Protocol.parse_network_list(data["data"])
            self._update_wifi_results(networks)
        elif "devices" in data.get("data", {}):
            # BLE scan results
            devices = Protocol.parse_ble_devices(data["data"])
            self._update_ble_results(devices)
        elif "packets_total" in data.get("data", {}):
            # Packet stats
            stats = Protocol.parse_packet_stats(data)
            self._update_packet_stats(stats)
    
    def _handle_status_update(self, data: Dict[str, Any]):
        """Handle status updates from device"""
        status = Protocol.parse_status(data)
        if status:
            self.device_status = status
            self._update_device_info()
    
    def _update_wifi_results(self, networks: list):
        """Update WiFi scan results"""
        self.wifi_networks = networks
        
        # Re-enable scan button, disable stop button and update status
        self.wifi_scan_btn.configure(state="normal")
        self.wifi_stop_btn.configure(state="disabled")
        self.status_label.configure(text="Ready")
        
        # Clear existing items
        for item in self.wifi_tree.get_children():
            self.wifi_tree.delete(item)
        
        # Add new items
        for net in networks:
            # Color based on signal strength
            if net.rssi > -50:
                tag = "strong"
            elif net.rssi > -70:
                tag = "medium"
            else:
                tag = "weak"
            
            self.wifi_tree.insert(
                "",
                "end",
                values=(
                    net.ssid or "(Hidden)",
                    net.bssid,
                    net.channel,
                    f"{net.rssi} dBm",
                    net.security
                ),
                tags=(tag,)
            )
        
        # Configure tags
        self.wifi_tree.tag_configure("strong", foreground=self.colors["success"])
        self.wifi_tree.tag_configure("medium", foreground=self.colors["warning"])
        self.wifi_tree.tag_configure("weak", foreground=self.colors["error"])
        
        # Update count
        self.wifi_count_label.configure(text=f"Networks: {len(networks)}")
        self.log_message(f"WiFi scan complete: {len(networks)} networks found", "success")
    
    def _update_device_info(self):
        """Update device info in status bar"""
        if self.device_status:
            info = f"Heap: {self.device_status.free_heap // 1024}KB | "
            info += f"Uptime: {self.device_status.uptime // 1000}s | "
            info += f"Battery: {self.device_status.battery_level}%"
            self.device_info_label.configure(text=info)
    
    def log_message(self, message: str, level: str = "info"):
        """Add message to console log"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        
        # Color based on level
        colors = {
            "info": self.colors["fg_secondary"],
            "success": self.colors["success"],
            "warning": self.colors["warning"],
            "error": self.colors["error"]
        }
        color = colors.get(level, self.colors["fg_primary"])
        
        # Add to console
        self.console_text.insert(
            "end",
            f"[{timestamp}] {message}\n"
        )
        self.console_text.see("end")
    
    # Event handlers
    def _on_connect_click(self):
        """Handle connect button click"""
        if not self.ble_controller.connected:
            self.connect_btn.configure(text="Connecting...", state="disabled")
            self.status_label.configure(text="Scanning for device...")
            
            # Run async connect
            asyncio.run_coroutine_threadsafe(
                self._async_connect(),
                self.async_loop
            )
        else:
            # Disconnect
            asyncio.run_coroutine_threadsafe(
                self._async_disconnect(),
                self.async_loop
            )
    
    async def _async_connect(self):
        """Async connect to device"""
        success = await self.ble_controller.connect()
        
        # Update UI in main thread
        self.root.after(0, self._update_connection_state, success)
    
    async def _async_disconnect(self):
        """Async disconnect from device"""
        await self.ble_controller.disconnect()
        self.root.after(0, self._update_connection_state, False)
    
    def _update_connection_state(self, connected: bool):
        """Update UI connection state"""
        if connected:
            self.conn_status_label.configure(
                text="● Connected",
                text_color=self.colors["success"]
            )
            self.connect_btn.configure(
                text="Disconnect",
                state="normal",
                fg_color=self.colors["error"]
            )
            self.status_label.configure(text="Connected to CyberTool")
            self.log_message("Connected to device", "success")
            
            # Get initial status
            asyncio.run_coroutine_threadsafe(
                self.ble_controller.get_status(),
                self.async_loop
            )
        else:
            self.conn_status_label.configure(
                text="● Disconnected",
                text_color=self.colors["error"]
            )
            self.connect_btn.configure(
                text="Connect",
                state="normal",
                fg_color=self.colors["purple"]
            )
            self.status_label.configure(text="Disconnected")
            self.device_info_label.configure(text="")
            
            if self.connect_btn.cget("text") == "Connecting...":
                self.log_message("Failed to connect to device", "error")
            else:
                self.log_message("Disconnected from device", "info")
    
    def _on_wifi_scan_click(self):
        """Handle WiFi scan button click"""
        if not self.ble_controller.connected:
            self.log_message("Not connected to device", "error")
            return
        
        self.wifi_scan_btn.configure(state="disabled")
        self.wifi_stop_btn.configure(state="normal")
        self.status_label.configure(text="Scanning WiFi networks...")
        self.log_message("Starting WiFi scan...", "info")
        
        # Clear previous results
        for item in self.wifi_tree.get_children():
            self.wifi_tree.delete(item)
        self.wifi_count_label.configure(text="Networks: 0")
        
        # Get channel
        channel = 0 if self.channel_var.get() == "All" else int(self.channel_var.get())
        
        # Run async scan
        self.wifi_scan_task = asyncio.run_coroutine_threadsafe(
            self._async_wifi_scan(channel),
            self.async_loop
        )
    
    def _on_wifi_stop_click(self):
        """Handle WiFi stop scan button click"""
        if hasattr(self, 'wifi_scan_task') and not self.wifi_scan_task.done():
            self.wifi_scan_task.cancel()
            self.log_message("Stopping WiFi scan...", "warning")
            
        self.wifi_scan_btn.configure(state="normal")
        self.wifi_stop_btn.configure(state="disabled")
        self.status_label.configure(text="WiFi scan stopped")
        
        # Send stop command to device (not implemented in firmware yet)
        # asyncio.run_coroutine_threadsafe(
        #     self.ble_controller.stop_scan(),
        #     self.async_loop
        # )
    
    async def _async_wifi_scan(self, channel: int):
        """Async WiFi scan"""
        response = await self.ble_controller.scan_wifi(channel=channel)
        
        if response:
            if response.get("status") == "error" or response.get("status") == ResponseStatus.TIMEOUT.value:
                self.root.after(0, self.log_message, "WiFi scan failed", "error")
                # Re-enable button on error
                self.root.after(0, self.wifi_scan_btn.configure, {"state": "normal"})
                self.root.after(0, self.wifi_stop_btn.configure, {"state": "disabled"})
                self.root.after(0, self.status_label.configure, {"text": "Ready"})
            elif response.get("status") == ResponseStatus.ERROR.value:
                error_msg = response.get("error", "Unknown error")
                self.root.after(0, self.log_message, f"WiFi scan error: {error_msg}", "error")
                # Re-enable button on error
                self.root.after(0, self.wifi_scan_btn.configure, {"state": "normal"})
                self.root.after(0, self.wifi_stop_btn.configure, {"state": "disabled"})
                self.root.after(0, self.status_label.configure, {"text": "Ready"})
        else:
            self.root.after(0, self.log_message, "WiFi scan failed: No response", "error")
            # Re-enable button on error
            self.root.after(0, self.wifi_scan_btn.configure, {"state": "normal"})
            self.root.after(0, self.wifi_stop_btn.configure, {"state": "disabled"})
            self.root.after(0, self.status_label.configure, {"text": "Ready"})
    
    def _on_ble_scan_click(self):
        """Handle BLE scan button click"""
        if not self.ble_controller.connected:
            self.log_message("Not connected to device", "error")
            return
        
        self.ble_scan_btn.configure(state="disabled")
        self.status_label.configure(text="Scanning Bluetooth devices...")
        self.log_message("Starting Bluetooth scan...", "info")
        
        # Run async scan
        asyncio.run_coroutine_threadsafe(
            self._async_ble_scan(),
            self.async_loop
        )
    
    async def _async_ble_scan(self):
        """Async BLE scan"""
        response = await self.ble_controller.scan_ble()
        
        if response and response.get("status") == "error":
            self.root.after(0, self.log_message, "Bluetooth scan failed", "error")
            # Re-enable button on error
            self.root.after(0, self.ble_scan_btn.configure, {"state": "normal"})
            self.root.after(0, self.status_label.configure, {"text": "Ready"})
    
    def _update_ble_results(self, devices: list):
        """Update BLE scan results"""
        self.ble_devices = devices
        self.ble_count_label.configure(text=f"Devices: {len(devices)}")
        
        # Re-enable scan button and update status
        self.ble_scan_btn.configure(state="normal")
        self.status_label.configure(text="Ready")
        
        self.log_message(f"Bluetooth scan complete: {len(devices)} devices found", "success")
    
    def _update_packet_stats(self, stats: Optional[PacketStats]):
        """Update packet monitoring statistics"""
        if stats:
            self.stat_labels["Total Packets"].configure(text=str(stats.packets_total))
            self.stat_labels["Packets/sec"].configure(text=str(stats.packets_per_sec))
            self.stat_labels["Beacons"].configure(text=str(stats.beacon_count))
            self.stat_labels["Probes"].configure(text=str(stats.probe_count))
            self.stat_labels["Data"].configure(text=str(stats.data_count))
            self.stat_labels["Management"].configure(text=str(stats.mgmt_count))
    
    def run(self):
        """Run the GUI application"""
        self.root.mainloop()