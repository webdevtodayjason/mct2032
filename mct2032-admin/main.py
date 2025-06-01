#!/usr/bin/env python3
"""
MCT2032 Admin Console
Main entry point for the application
"""

import sys
import logging
import asyncio
from mct2032_admin import CyberToolGUI


def setup_logging():
    """Configure logging for the application"""
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.StreamHandler(sys.stdout),
            logging.FileHandler('mct2032_admin.log')
        ]
    )
    
    # Reduce noise from bleak
    logging.getLogger("bleak").setLevel(logging.WARNING)


def main():
    """Main application entry point"""
    print("""
╔═══════════════════════════════════════════════╗
║          MCT2032 - Mini Cyber Tool 2032       ║
║              Admin Console v1.0.0             ║
╚═══════════════════════════════════════════════╝
    """)
    
    # Setup logging
    setup_logging()
    logger = logging.getLogger(__name__)
    
    logger.info("Starting MCT2032 Admin Console...")
    
    try:
        # Create and run GUI
        app = CyberToolGUI()
        app.run()
        
    except KeyboardInterrupt:
        logger.info("Application terminated by user")
        sys.exit(0)
    except Exception as e:
        logger.error(f"Fatal error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()