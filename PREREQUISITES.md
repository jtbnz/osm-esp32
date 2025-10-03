# Prerequisites - User Setup Tasks

This document outlines the tasks **you** need to complete before/during development. These involve creating accounts and obtaining credentials for both mining modes.

---

## 1. ESP-IDF Installation

### Install ESP-IDF v5.1 or later

**macOS** (your platform):
```bash
# Install prerequisites
brew install cmake ninja dfu-util python3

# Create esp directory
mkdir -p ~/esp
cd ~/esp

# Clone ESP-IDF
git clone -b v5.1 --recursive https://github.com/espressif/esp-idf.git

# Run installer
cd esp-idf
./install.sh esp32s3

# Add to shell profile (add to ~/.zshrc or ~/.bash_profile)
echo 'alias get_idf=". $HOME/esp/esp-idf/export.sh"' >> ~/.zshrc

# Activate ESP-IDF (run this in each new terminal)
. $HOME/esp/esp-idf/export.sh
```

**Verify installation**:
```bash
idf.py --version
# Should show: ESP-IDF v5.1.x or later
```

---

## 2. Bitcoin Setup

### 2.1 Create Bitcoin Wallet

You need a Bitcoin wallet address to receive rewards (if you ever win the lottery!).

**Recommended Wallets**:

**Option A: Electrum (Desktop) - RECOMMENDED**
1. Download from: https://electrum.org/
2. Install and launch
3. Create new wallet
4. **IMPORTANT**: Write down your 12-word seed phrase on paper
   - Store it somewhere safe
   - Never share it with anyone
   - Never store it digitally (no screenshots, no cloud)
5. Once wallet is created, go to "Receive" tab
6. Copy your Bitcoin address (starts with `bc1...` or `3...`)
7. Save this address - you'll need it for configuration

**Option B: BlueWallet (Mobile)**
1. Download from App Store (iOS) or Play Store (Android)
2. Create new Bitcoin wallet
3. Write down seed phrase (same security as above)
4. Tap "Receive" to see your address
5. Copy and save the address

**What you need**:
- ✅ Bitcoin wallet address (starts with `bc1`, `3`, or `1`)
- ✅ Seed phrase backed up securely (NEVER commit to git!)

### 2.2 Choose Bitcoin Solo Pool

You need a solo mining pool to connect to. These pools allow you to mine Bitcoin with a tiny chance of finding a block.

**Recommended Pools**:

**Option A: public-pool.io (RECOMMENDED)**
- URL: `public-pool.io`
- Port: `21496` (solo mining)
- No registration required
- Dashboard: https://web.public-pool.io/
- Documentation: https://web.public-pool.io/#/help

**Option B: solo.ckpool.org**
- URL: `solo.ckpool.org`
- Port: `3333`
- No registration required
- Smaller, friendly community
- Dashboard: https://solo.ckpool.org/

**What you need**:
- ✅ Pool URL (e.g., `public-pool.io`)
- ✅ Pool port (e.g., `21496`)
- ✅ Worker name (can be anything, e.g., `ESP32-Miner-01`)

### 2.3 Understanding Bitcoin Lottery Mining

**IMPORTANT EXPECTATIONS**:
- Your ESP32 will hash at ~45,000 H/s (45 KH/s)
- Bitcoin network hashes at ~400,000,000,000,000,000 H/s (400 EH/s)
- Your probability of finding a block: **~0.00000001%**
- Expected time to find a block: **~31 billion years**

**BUT**:
- The chance is NON-ZERO (unlike winning without a ticket)
- It's like a perpetual lottery ticket
- Educational value is HIGH
- If you win, block reward is 6.25 BTC (~$250,000 at current prices)

**Think of it as**:
- Educational project to understand Bitcoin mining
- Lottery ticket that costs only electricity (~$0.01/day)
- Cool talking piece ("I'm solo mining Bitcoin on an ESP32!")

---

## 3. Duino-Coin Setup

### 3.1 Create Duino-Coin Account

Duino-Coin is designed for low-power devices and you WILL earn coins!

**Steps**:

1. **Visit Duino-Coin website**: https://duinocoin.com/

2. **Click "Get Started"**

3. **Create account**:
   - Username: Choose a username (e.g., `yourname`)
   - Password: Choose a strong password
   - Email: Provide email (for password recovery)
   - Click "Sign Up"

4. **Verify email**: Check your email and verify your account

5. **Login to web wallet**: https://wallet.duinocoin.com/
   - Login with your username and password
   - This is where you'll see your earnings

6. **Get mining key (OPTIONAL but recommended)**:
   - Go to Settings in web wallet
   - Click "Generate Mining Key" or "Show Mining Key"
   - Copy this key (it's like a password for mining)
   - Provides extra security for your mining

**What you need**:
- ✅ Duino-Coin username
- ✅ Duino-Coin mining key (optional, but recommended)
- ✅ Password (for web wallet access)

### 3.2 Understanding Duino-Coin Mining

**REALISTIC EXPECTATIONS**:
- Your ESP32 will hash at ~30,000 H/s (30 KH/s)
- You WILL earn coins within the first day
- Expected earnings: **~1-5 DUCO per day**
- Current value: Very low (~$0.001-0.01 per DUCO, varies)

**What this means**:
- You'll earn $0.001-0.05 per day (not profitable, but REAL)
- You'll see the complete mining cycle (shares, rewards, balance)
- Understand how mining actually works
- Learn about mining pools and reward systems
- Get instant feedback (not 31 billion years!)

**Duino-Coin Features**:
- Kolka system: Designed to reward low-power devices fairly
- ESP32 gets appropriate difficulty (you can actually solve shares)
- Web wallet: Track earnings in real-time
- Mobile app: Monitor mining from phone
- Community: Active Discord and forums

### 3.3 Install Duino-Coin Mobile App (OPTIONAL)

**iOS**: Search "Duino-Coin" in App Store
**Android**: Search "Duino-Coin" in Play Store

This lets you monitor your ESP32 miner from your phone and see earnings in real-time.

---

## 4. WiFi Information

You'll need WiFi credentials for your ESP32 to connect.

**What you need**:
- ✅ WiFi SSID (network name)
- ✅ WiFi password
- ✅ 2.4 GHz network (ESP32 doesn't support 5 GHz)

**Note**: The device will create an AP mode fallback if it can't connect:
- AP SSID: `ESP32-Miner-Setup`
- AP Password: `duino123`
- AP IP: `192.168.4.1`
- You can configure WiFi through web interface in AP mode

---

## 5. Hardware Checklist

Make sure you have:
- ✅ ESP32-S3 board with 7-inch display (VIEWESMART UEDX80480070ESP32)
- ✅ USB cable (USB-C, for flashing and power)
- ✅ Computer with USB port
- ✅ 5V power supply (can use USB, but 2A+ recommended for display)

---

## 6. Configuration Summary

Once you have everything, you'll create a `config.h` file with these values:

```c
// WiFi Configuration
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourWiFiPassword"

// Bitcoin Configuration
#define BTC_POOL_URL "public-pool.io"
#define BTC_POOL_PORT 21496
#define BTC_WALLET_ADDRESS "bc1q..." // Your Bitcoin address
#define BTC_WORKER_NAME "ESP32-Miner-01"

// Duino-Coin Configuration
#define DUCO_USERNAME "yourusername"
#define DUCO_MINING_KEY "your_mining_key_here" // Optional
#define DUCO_SERVER "server.duinocoin.com"
#define DUCO_PORT 2811

// Default mining mode (0 = Bitcoin, 1 = Duino-Coin)
#define DEFAULT_MINING_MODE 1 // Start with DUCO to see results quickly!
```

**IMPORTANT**: Never commit this file to git! It's in `.gitignore`.

---

## 7. Checklist Before Starting

Before I begin implementation, make sure you have:

### Bitcoin Setup
- [ ] Bitcoin wallet created (Electrum or BlueWallet)
- [ ] Bitcoin address copied and saved
- [ ] Seed phrase written down and stored securely
- [ ] Solo pool selected (public-pool.io recommended)
- [ ] Worker name chosen

### Duino-Coin Setup
- [ ] Duino-Coin account created at duinocoin.com
- [ ] Email verified
- [ ] Username noted
- [ ] Mining key generated (optional but recommended)
- [ ] Web wallet accessible at wallet.duinocoin.com
- [ ] Mobile app installed (optional)

### Development Environment
- [ ] ESP-IDF v5.1+ installed and working
- [ ] `idf.py --version` shows correct version
- [ ] USB drivers installed (usually automatic)
- [ ] Serial port permissions (may need on Linux: `sudo usermod -a -G dialout $USER`)

### Hardware
- [ ] ESP32-S3 display board available
- [ ] USB cable connected
- [ ] Device powers on

### Network
- [ ] WiFi SSID known
- [ ] WiFi password known
- [ ] WiFi is 2.4 GHz (not 5 GHz)

---

## 8. Getting Started

Once you've completed the checklist above, you're ready! I'll start implementing:

1. **Phase 1**: Project structure and build system
2. **Phase 2**: Configuration system (I'll create `config.h.example` template)
3. **Phase 3**: WiFi manager with AP fallback
4. And continue through all 13 phases...

---

## 9. Useful Resources

### Bitcoin
- **Electrum Wallet**: https://electrum.org/
- **BlueWallet**: https://bluewallet.io/
- **public-pool.io**: https://web.public-pool.io/
- **solo.ckpool.org**: https://solo.ckpool.org/
- **Bitcoin Explorer**: https://blockchair.com/bitcoin (track your attempts)

### Duino-Coin
- **Website**: https://duinocoin.com/
- **Web Wallet**: https://wallet.duinocoin.com/
- **Getting Started Guide**: https://duinocoin.com/getting-started.html
- **GitHub**: https://github.com/revoxhere/duino-coin
- **Discord**: https://discord.gg/duinocoin (active community)

### ESP32 Development
- **ESP-IDF Docs**: https://docs.espressif.com/projects/esp-idf/en/latest/
- **LVGL Docs**: https://docs.lvgl.io/
- **Hardware Repo**: https://github.com/VIEWESMART/UEDX80480070ESP32-7inch-Touch-Display

---

## 10. Timeline

**Your tasks**: 30-60 minutes
- Create accounts
- Install ESP-IDF
- Gather credentials

**My tasks**: 3-4 weeks (105-140 hours)
- Implement all 13 phases
- Complete testing
- Documentation

---

## Questions?

If you have questions about any of these steps, let me know before I start coding!

**Ready to proceed?**

Once you've completed the checklist, say:
- "I've completed the prerequisites, start Phase 1" or
- "Start implementing" or
- Ask any questions you have!

---

## Security Reminders

🔒 **NEVER share or commit**:
- Bitcoin seed phrase (12-24 words)
- Bitcoin wallet private keys
- Duino-Coin password
- WiFi password
- Mining keys

✅ **Safe to share**:
- Bitcoin wallet address (public, like an email)
- Duino-Coin username
- Pool URLs
- Worker names

🔐 **The `config.h` file will be gitignored** - your secrets stay local!
