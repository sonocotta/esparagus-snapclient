# Esparagus Snapclient web-installer for ESP32

A synchronous multiroom audio streaming client for [Snapcast](https://github.com/CarlosDerSeher/snapclient) designed specifically for ESP32-based audio devices, with particular focus on the [Sonocotta Esparagus](https://www.tindie.com/stores/sonocotta/) series of audio boards.

## Snapclient Features

- **High-Quality Audio**: FLAC, OPUS, and PCM decoding with sample rates up to 48kHz
- **Multiroom Sync**: Perfect synchronization across multiple devices
- **WiFi Provisioning**: Easy setup via Improv WiFi or manual configuration
- **Auto-Discovery**: Automatic Snapcast server detection via mDNS
- **Web Interface**: HTTP server for device configuration and control (very basic at this moment)
- **Multiple Board Support**: Extensive support for various ESP32 audio boards

## Project Structure

```text
├── snapclient/          # Main ESP32 snapcast client firmware
├── installer/           # Web-based firmware installer
├── configs/             # Pre-built configurations for different boards
├── configs-test/        # Test configurations
├── docs/                # Documentation and web installer interface
│   └── artifacts/       # Pre-built firmware binaries and manifests
└── build-local.sh       # Local build script
```

## Supported Hardware

### Sonocotta Esparagus Series

- **HiFi Esparagus**: PCM5100 DAC with line-level output
- **Loud Esparagus**: Dual MAX98357 amplifiers (3W per channel)
- **Amped Esparagus**: PCM5100 DAC + TPA3110D2 amplifier
- **Louder Esparagus**: TAS5805M DAC with 25W per channel output

### Sonocotta ESP32 Audio Boards

- **HiFi-ESP32**: Cost-efficient dev board with PCM5100 DAC with line-level output
- **Loud-ESP32**: Cost-efficient dev board with dual MAX98357 amplifiers (3W per channel)
- **Amped-ESP32**: Cost-efficient dev board with PCM5100 DAC + TPA3110/TPA3128 amplifier
- **Louder-ESP32**: Cost-efficient dev board with TAS5805M-based high-power DAC

### Espressif Development Boards

- **ESP32-LyraT v4.2/v4.3**: Official Espressif audio development board
- **ESP32-LyraT-Mini**: Compact version of LyraT

### AI-Thinker Boards

- **ESP32-Audio-Kit**: ES8388-based development board

## Quick Start

### Option 1: Web Installer (Recommended)

1. **Visit the Web Installer**: Open [esparagus-snapclient](https://sonocotta.github.io/esparagus-snapclient/) in a Chrome/Edge browser
2. **Select Your Device**: Choose your board from the supported devices list
3. **Connect**: Plug in your ESP32 device via USB
4. **Install**: Click install and follow the prompts
5. **Configure**: Set up WiFi via browser ([Wifi-Improv](https://github.com/improv-wifi))

### Option 2: Pre-built Binaries

1. Download the appropriate firmware files from `docs/artifacts/binaries/`
2. Flash using esptool with all required partitions:

   ```bash
   esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash \
     0x1000 your-device-bootloader.bin \
     0x8000 your-device-partition-table.bin \
     0xd000 your-device-ota_data_initial.bin \
     0x10000 your-device-snapclient.bin \
     0x370000 your-device-storage.bin
   ```

   For example, for HiFi-ESP32:

   ```bash
   esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash \
     0x1000 hifi-esp32-snapclient-2025-11-14-bootloader.bin \
     0x8000 hifi-esp32-snapclient-2025-11-14-partition-table.bin \
     0xd000 hifi-esp32-snapclient-2025-11-14-ota_data_initial.bin \
     0x10000 hifi-esp32-snapclient-2025-11-14-snapclient.bin \
     0x370000 hifi-esp32-snapclient-2025-11-14-storage.bin
   ```

### Option 3: Build from Source

#### Prerequisites

- Docker (recommended) or ESP-IDF v5.1.5
- Git with submodules support

#### Build Steps

1. **Clone the repository**:

   ```bash
   git clone https://github.com/sonocotta/esparagus-snapclient-new.git
   cd esparagus-snapclient-new
   git submodule update --init --recursive
   ```

2. **Build all configurations** (Docker):

   ```bash
   ./build-local.sh
   ```

3. **Build single configuration**:

   ```bash
   # Using Docker
   docker run --rm -v ./snapclient:/project -v ./configs:/project/configs -w /project espressif/idf:v5.1.1 /bin/bash -c '. /opt/esp/idf/export.sh && cp configs/sdkconfig.your-device sdkconfig && idf.py build'
   
   # Or with local ESP-IDF
   cd snapclient
   cp configs/sdkconfig.your-device sdkconfig
   idf.py build flash monitor
   ```

## Configuration

### Available Configurations

The `configs/` directory contains pre-configured settings for various boards:

- `sdkconfig.hifi-esparagus` - HiFi Esparagus board
- `sdkconfig.loud-esparagus` - Loud Esparagus board  
- `sdkconfig.amped-esparagus` - Amped Esparagus board
- `sdkconfig.louder-esparagus` - Louder Esparagus board
- `sdkconfig.lyra-t-4.3` - ESP32-LyraT v4.3
- And few more...

### Custom Configuration

To create a custom configuration:

1. Copy a similar configuration: `cp configs/sdkconfig.template configs/sdkconfig.custom`
2. Modify using menuconfig: `idf.py menuconfig`
3. Configure your specific hardware settings:
   - **Audio HAL**: Choose your audio board
   - **Custom Audio Board**: Configure DAC and GPIO pins
   - **DSP Processor**: Audio effects and EQ settings
   - **WiFi**: Network credentials or provisioning
   - **Snapclient**: Server settings and client name

## Web Interface

The device exposes a web interface for configuration and control:

1. Connect to the device's WiFi network or find its IP address
2. Open `http://device-ip` in your browser
3. Configure audio settings, view status, and control playback

## Development

### VS Code Tasks

The project includes pre-configured VS Code tasks:

- **Build All Configurations**: Builds firmware for all supported devices
- **Build Single Configuration**: Builds firmware for a specific device
- **Serve HTML**: Runs the device web server
- **Serve Installer**: Runs the installer development server

### Web Installer Development

The installer is a modern web application built with:

- Webpack for bundling
- Bootstrap for UI
- Improv WiFi SDK for device communication

To develop the installer:

```bash
cd installer
npm install
npm run dev
```

## Audio Processing

### DSP Features (to be implemented in the UI)

- **Stereo/Mono modes**: Flexible channel routing
- **Equalizer**: Configurable frequency response
- **Crossover filters**: For bi-amp and subwoofer setups
- **Volume control**: Software or hardware-based
- **Loudness processing**: Multiple EQ presets

## Integration

### Snapcast Server Setup

1. Install Snapcast server on your network
2. Configure audio sources (e.g., streaming services, local files)
3. ESP32 clients will auto-discover and connect
4. Control multi-room audio via Snapcast clients

Example server configuration:

```bash
snapserver -s pipe:///tmp/snapfifo?name=Music&sampleformat=48000:16:2
```

### Home Assistant Integration

The devices can be integrated with Home Assistant / Music Assistant using:

- HTTP API for configuration
- mDNS for automatic discovery

## Troubleshooting

### Common Issues

1. **WiFi Connection Problems**: Use Improv WiFi for easier setup
2. **Audio Dropouts**: Check network stability and buffer settings
3. **Sync Issues**: Ensure all devices have stable network connections
4. **Build Errors**: Verify ESP-IDF version (v5.1.5) and submodules

### Debug Features

- Serial monitor output for diagnostics, also available via web-installer UI
- Web interface status page
- HTTP API for device information

## Task List

- [ ] Add device settings store on the NVS
- [ ] Provide single binary, and multiple NVS binaries for devices
- [ ] Add self-hosted UI page for device settings
- [ ] Device settings: name, pinouts
- [ ] Device settings: DAC settings
- [ ] device settings: DSP presets (PCM5122, TAS5805M, TAS5825M)

## Links

- [Snapcast ESP32 port](https://github.com/CarlosDerSeher/snapclient) (active development)
- [Snapclient ESPHome component](https://github.com/c-MM/esphome-snapclient/) (experimental)
- [Sonocotta Hardware](https://www.tindie.com/stores/sonocotta/)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
