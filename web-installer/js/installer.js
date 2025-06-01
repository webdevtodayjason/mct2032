/**
 * MCT2032 Web Installer JavaScript
 * Matrix rain effect and interactive features
 */

// Matrix Rain Effect
class MatrixRain {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.fontSize = 14;
        this.columns = 0;
        this.drops = [];
        this.chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*()_+-=[]{}|;:,.<>?~';
        
        this.resize();
        this.init();
        
        window.addEventListener('resize', () => this.resize());
    }
    
    resize() {
        this.canvas.width = window.innerWidth;
        this.canvas.height = window.innerHeight;
        this.columns = Math.floor(this.canvas.width / this.fontSize);
        
        // Initialize drops
        for (let i = 0; i < this.columns; i++) {
            if (!this.drops[i]) {
                this.drops[i] = Math.random() * -100;
            }
        }
    }
    
    init() {
        this.ctx.fillStyle = 'rgba(10, 14, 39, 0.05)';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    }
    
    draw() {
        // Fade effect
        this.ctx.fillStyle = 'rgba(10, 14, 39, 0.05)';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        // Set text properties
        this.ctx.fillStyle = '#9333ea';
        this.ctx.font = `${this.fontSize}px monospace`;
        
        // Draw characters
        for (let i = 0; i < this.drops.length; i++) {
            const char = this.chars[Math.floor(Math.random() * this.chars.length)];
            const x = i * this.fontSize;
            const y = this.drops[i] * this.fontSize;
            
            // Gradient effect
            const gradient = this.ctx.createLinearGradient(0, y - 20, 0, y);
            gradient.addColorStop(0, 'rgba(147, 51, 234, 0)');
            gradient.addColorStop(0.5, 'rgba(147, 51, 234, 0.5)');
            gradient.addColorStop(1, 'rgba(147, 51, 234, 1)');
            
            this.ctx.fillStyle = gradient;
            this.ctx.fillText(char, x, y);
            
            // Reset drop when it reaches bottom
            if (y > this.canvas.height && Math.random() > 0.975) {
                this.drops[i] = 0;
            }
            
            this.drops[i]++;
        }
    }
    
    animate() {
        this.draw();
        requestAnimationFrame(() => this.animate());
    }
}

// Initialize Matrix Rain
document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById('matrix-rain');
    const matrixRain = new MatrixRain(canvas);
    matrixRain.animate();
    
    // Smooth scroll for anchor links
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            const target = document.querySelector(this.getAttribute('href'));
            if (target) {
                target.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
            }
        });
    });
    
    // Install button enhancements
    const installButton = document.querySelector('esp-web-install-button');
    if (installButton) {
        installButton.addEventListener('click', () => {
            console.log('Starting firmware installation...');
        });
        
        // Listen for installation events
        installButton.addEventListener('state-changed', (e) => {
            console.log('Installation state:', e.detail);
            
            // Update UI based on state
            const button = installButton.querySelector('.cyber-button');
            const buttonText = button.querySelector('.button-text');
            
            switch (e.detail.state) {
                case 'initializing':
                    buttonText.textContent = 'Initializing...';
                    button.classList.add('loading');
                    break;
                case 'manifest':
                    buttonText.textContent = 'Loading Firmware...';
                    break;
                case 'downloading':
                    buttonText.textContent = 'Downloading...';
                    break;
                case 'writing':
                    buttonText.textContent = 'Writing Firmware...';
                    break;
                case 'finished':
                    buttonText.textContent = 'Installation Complete!';
                    button.classList.remove('loading');
                    button.classList.add('success');
                    setTimeout(() => {
                        buttonText.textContent = 'Install Firmware';
                        button.classList.remove('success');
                    }, 5000);
                    break;
                case 'error':
                    buttonText.textContent = 'Installation Failed';
                    button.classList.remove('loading');
                    button.classList.add('error');
                    setTimeout(() => {
                        buttonText.textContent = 'Install Firmware';
                        button.classList.remove('error');
                    }, 5000);
                    break;
            }
        });
    }
    
    // Intersection Observer for animations
    const observerOptions = {
        threshold: 0.1,
        rootMargin: '0px 0px -100px 0px'
    };
    
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('animate-in');
            }
        });
    }, observerOptions);
    
    // Observe elements
    document.querySelectorAll('.feature-card, .step, .spec-item').forEach(el => {
        observer.observe(el);
    });
    
    // Parallax effect for hero section
    let ticking = false;
    function updateParallax() {
        const scrolled = window.pageYOffset;
        const parallaxElements = document.querySelectorAll('.logo-glow, .device-frame');
        
        parallaxElements.forEach(el => {
            const rate = el.classList.contains('logo-glow') ? 0.5 : 0.3;
            el.style.transform = `translateY(${scrolled * rate}px)`;
        });
        
        ticking = false;
    }
    
    function requestTick() {
        if (!ticking) {
            requestAnimationFrame(updateParallax);
            ticking = true;
        }
    }
    
    window.addEventListener('scroll', requestTick);
    
    // Add glitch effect on hover
    const logo = document.querySelector('.logo');
    if (logo) {
        logo.addEventListener('mouseenter', () => {
            logo.classList.add('glitch');
            setTimeout(() => {
                logo.classList.remove('glitch');
            }, 300);
        });
    }
    
    // Browser compatibility check
    function checkBrowserCompatibility() {
        const isChrome = /Chrome/.test(navigator.userAgent) && /Google Inc/.test(navigator.vendor);
        const isEdge = /Edg/.test(navigator.userAgent);
        const hasSerial = 'serial' in navigator;
        
        if (!hasSerial || (!isChrome && !isEdge)) {
            const browserNote = document.querySelector('.browser-note');
            if (browserNote) {
                browserNote.innerHTML = '⚠️ Web Serial API not supported. Please use Chrome or Edge browser.';
                browserNote.style.color = 'var(--error)';
            }
            
            const installButton = document.querySelector('.cyber-button');
            if (installButton) {
                installButton.disabled = true;
                installButton.style.opacity = '0.5';
                installButton.style.cursor = 'not-allowed';
            }
        }
    }
    
    checkBrowserCompatibility();
    
    // Console Easter egg
    console.log('%c🔐 MCT2032 - Mega Cyber Tool 2032 🔐', 'font-size: 20px; font-weight: bold; color: #9333ea;');
    console.log('%cWelcome to the matrix...', 'font-size: 14px; color: #a855f7;');
    console.log('%cFor authorized security testing only!', 'font-size: 12px; color: #ef4444;');
});

// Add CSS animations dynamically
const style = document.createElement('style');
style.textContent = `
    @keyframes glitch {
        0%, 100% { 
            text-shadow: 
                0 0 40px rgba(168, 85, 247, 0.5),
                -2px 0 #ff00ff,
                2px 0 #00ffff;
        }
        20% { 
            text-shadow: 
                0 0 40px rgba(168, 85, 247, 0.5),
                2px 0 #ff00ff,
                -2px 0 #00ffff;
        }
        40% { 
            text-shadow: 
                0 0 40px rgba(168, 85, 247, 0.5),
                -2px 0 #ff00ff,
                2px 0 #00ffff;
        }
    }
    
    .glitch {
        animation: glitch 0.3s ease-in-out;
    }
    
    .animate-in {
        animation: fadeInUp 0.6s ease-out forwards;
    }
    
    @keyframes fadeInUp {
        from {
            opacity: 0;
            transform: translateY(30px);
        }
        to {
            opacity: 1;
            transform: translateY(0);
        }
    }
    
    .cyber-button.loading .button-text::after {
        content: '';
        display: inline-block;
        width: 10px;
        height: 10px;
        margin-left: 10px;
        border: 2px solid transparent;
        border-top-color: currentColor;
        border-radius: 50%;
        animation: spin 1s linear infinite;
    }
    
    @keyframes spin {
        to { transform: rotate(360deg); }
    }
    
    .cyber-button.success {
        background: linear-gradient(135deg, var(--success) 0%, #059669 100%);
        border-color: var(--success);
    }
    
    .cyber-button.error {
        background: linear-gradient(135deg, var(--error) 0%, #dc2626 100%);
        border-color: var(--error);
    }
`;
document.head.appendChild(style);