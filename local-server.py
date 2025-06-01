#!/usr/bin/env python3
"""
Local web server for testing MCT2032 web installer
Serves files with proper CORS headers for Web Serial API
"""

import http.server
import socketserver
import os
import sys

PORT = 8000

class CORSHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """HTTP request handler with CORS headers for Web Serial API"""
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory="web-installer", **kwargs)
    
    def end_headers(self):
        # Add CORS headers required for Web Serial API
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', '*')
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        super().end_headers()
    
    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    
    print(f"""
╔═══════════════════════════════════════════════╗
║       MCT2032 Local Web Server                ║
╚═══════════════════════════════════════════════╝

Starting local server on http://localhost:{PORT}

Important: The Web Serial API requires HTTPS in production,
but Chrome/Edge allow it on localhost for testing.

Press Ctrl+C to stop the server.
""")
    
    with socketserver.TCPServer(("", PORT), CORSHTTPRequestHandler) as httpd:
        print(f"Server running at http://localhost:{PORT}/")
        print(f"Open http://localhost:{PORT}/ in Chrome or Edge browser")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped.")
            sys.exit(0)

if __name__ == "__main__":
    main()