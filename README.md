# Waste Collection Schedule Display

A project for JC2432W328 variant of Cheap Yellow Display that shows upcoming waste collection schedules on a 240x320 ST7789 display using LittleVGL for the graphical interface.

## Features

- **WiFi Connectivity**: Connects to WiFi networks and saves credentials for automatic reconnection
- **Data Fetching**: Retrieves collection schedules from a local web server
- **Automatic Refresh**: Updates data every 12 hours automatically
- **Color-Coded Display**: Different collection types shown in different colors:
  - Food Waste: Red
  - Garden Waste: Light Green  
  - Recycling: Blue
  - General Waste: White (default)
- **Persistent Storage**: WiFi credentials saved using ESP32 Preferences
- **Touch Support**: CST820 capacitive touchscreen driver (touch functionality available)

## Hardware Requirements

- ESP32 development board
- 240x320 ST7789 TFT display
- CST820 capacitive touch controller (optional, for touch input)
- Power supply (5V recommended)

### Pin Connections

| Component | ESP32 Pin | Description |
|-----------|-----------|-------------|
| ST7789 SCLK | GPIO 14 | SPI clock |
| ST7789 MOSI | GPIO 13 | SPI data |
| ST7789 DC | GPIO 2 | Data/Command |
| ST7789 CS | GPIO 15 | Chip select |
| CST820 SDA | GPIO 21 | I2C data (touch) |
| CST820 SCL | GPIO 22 | I2C clock (touch) |
| CST820 RST | GPIO 18 | Touch reset |
| CST820 IRQ | GPIO 19 | Touch interrupt |
| Display Backlight | GPIO 27 | Display enable |

## Software Requirements

- Arduino IDE or PlatformIO
- ESP32 board support package
- Required libraries:
  - LovyanGFX
  - LittleVGL (lvgl)
  - WiFi
  - HTTPClient
  - Preferences

## Installation

1. **Clone or download the project files**
   ```
   git clone [repository-url]
   ```

2. **Install required libraries**
   - Install via Arduino Library Manager:
     - LovyanGFX by lovyan03
     - lvgl by LVGL
     - WiFi (included with ESP32)
     - HTTPClient (included with ESP32)
     - Preferences (included with ESP32)

3. **Configure the project**
   - Update WiFi credentials in the sketch or use the default:
     - SSID: `skynet`
     - Password: `ukstokes.com`
   - Update the server URL in `fetchCollectionsData()` function if needed:
     ```cpp
     String url = "http://192.168.1.131:9090/collections.txt";
     ```

4. **Upload to ESP32**
   - Select ESP32 board in Arduino IDE
   - Set correct COM port
   - Upload the sketch

## Data Format

The project expects a text file served at the configured URL like the included collections.txt

## Configuration

### LittleVGL Configuration (lv_conf.h)
Optimized for minimal memory usage (32KB heap):
- Only essential widgets enabled (Label, Display)
- Performance monitoring disabled
- Logging disabled
- 16-bit color depth

### Display Settings
- Resolution: 320x240 (rotated)
- SPI frequency: 27MHz write, 16MHz read
- Display buffer: 240x10 pixels (partial rendering)

## Usage

1. **First Boot**: The device will attempt to connect to the configured WiFi network
2. **Data Display**: Once connected, it fetches and displays the collection schedule
3. **Automatic Updates**: Data refreshes every 12 hours
4. **WiFi Reconnection**: If WiFi drops, it automatically attempts to reconnect

### Manual Refresh
The device checks the refresh condition every 30 seconds (to avoid excessive CPU usage) but only actually fetches new data when 12 hours have passed since the last successful fetch. To force a refresh:
1. Power cycle the device
2. Or wait for the 12-hour automatic refresh

## Memory Optimization

This project is optimized for ESP32 memory constraints:
- Limited LittleVGL features enabled
- Small display buffer (partial rendering)
- No unnecessary widgets or animations
- String handling optimized to avoid fragmentation

## Troubleshooting

### Common Issues

1. **WiFi Connection Failed**
   - Check SSID and password
   - Verify WiFi signal strength
   - Check if the network requires portal authentication

2. **No Data Displayed**
   - Verify server URL is accessible from the ESP32
   - Check serial monitor for HTTP error codes
   - Ensure data file format is correct

3. **Display Not Working**
   - Verify pin connections
   - Check power supply (display may need 5V)
   - Verify backlight pin (GPIO 27) is HIGH

4. **Memory Issues**
   - Reduce `LV_MEM_SIZE` in lv_conf.h if experiencing crashes
   - Disable additional features if needed

### Serial Monitor Output
Enable Serial monitor at 115200 baud for debugging:
- Connection status
- HTTP request results
- Data parsing information
- Refresh notifications

## Customization

### Changing Colors
Edit the `showCollectionsTable()` function in the main sketch:
```cpp
if (strstr(collections[i].type, "Food Waste") != NULL) {
  type_color = lv_color_hex(0xFF0000); // red
} else if (strstr(collections[i].type, "Garden Waste") != NULL) {
  type_color = lv_color_hex(0x90EE90); // light green
}
```

### Adding New Collection Types
Add additional `else if` conditions for new collection types with desired colors.

### Changing Refresh Interval
Modify `REFRESH_INTERVAL` in the main sketch:
```cpp
const unsigned long REFRESH_INTERVAL = 12 * 3600 * 1000UL; // 12 hours in milliseconds
```

## License

This project is open source. Feel free to modify and distribute.
