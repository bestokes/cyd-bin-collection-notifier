#include <LovyanGFX.hpp>
#include <lvgl.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
Preferences prefs;

// Data structures for collections
struct CollectionEntry {
  char date[11];      // DD/MM/YYYY + null
  char type[21];      // up to 20 chars + null
};

#define MAX_ENTRIES 5
CollectionEntry collections[MAX_ENTRIES];
int collectionCount = 0;

// Refresh timer
unsigned long lastFetchTime = 0;
const unsigned long REFRESH_INTERVAL = 12 * 3600 * 1000UL; // 12 hours in milliseconds

// Function prototypes
bool fetchCollectionsData();
void showCollectionsTable();
void attemptRefresh();

// App info removed to save space

// ======= Display Setup =======
class LGFX_JustDisplay : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;
public:
  LGFX_JustDisplay() {
    auto cfg = _bus.config();
    cfg.spi_host = VSPI_HOST;
    cfg.spi_mode = 0;
    cfg.freq_write = 27000000;
    cfg.freq_read  = 16000000;
    cfg.spi_3wire  = false;
    cfg.use_lock   = true;
    cfg.dma_channel= 1;
    cfg.pin_sclk   = 14;
    cfg.pin_mosi   = 13;
    cfg.pin_miso   = -1;
    cfg.pin_dc     = 2;
    _bus.config(cfg);
    _panel.setBus(&_bus);

    auto p_cfg = _panel.config();
    p_cfg.pin_cs          = 15;
    p_cfg.pin_rst         = -1;
    p_cfg.pin_busy        = -1;
    p_cfg.panel_width     = 240;
    p_cfg.panel_height    = 320;
    p_cfg.offset_rotation = 0;
    p_cfg.dummy_read_pixel= 8;
    p_cfg.dummy_read_bits = 1;
    _panel.config(p_cfg);
    setPanel(&_panel);
  }
};

LGFX_JustDisplay tft;

#define LVGL_TICK_PERIOD 5
static unsigned long lastLvTick = 0;

// LVGL flush callback
enum { DISP_W = 320, DISP_H = 240 };
void lv_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* color_p) {
  tft.pushImage(area->x1, area->y1,
                area->x2 - area->x1 + 1,
                area->y2 - area->y1 + 1,
                (lgfx::rgb565_t*)color_p);
  lv_display_flush_ready(disp);
}

// Touch input callback removed to save space

// UI objects
lv_obj_t* status_label; // global or static

// Welcome screen removed to save space

// Wi-Fi selection and connecting screens removed to save space


void start_wifi_connection(const char* ssid, const char* password) {
  Serial.printf("Actually starting connection to: %s\n", ssid);

  WiFi.begin(ssid, password);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
      Serial.println(F("\nConnected to Wi-Fi."));
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());

      // ✅ Save credentials now that we know they work
      prefs.begin("wifi", false); // RW mode
      prefs.putString("ssid", ssid);
      prefs.putString("pass", password);
      prefs.end();

      // After successful connection, fetch and display collections data
      delay(1000); // Small delay before fetching
      if (fetchCollectionsData()) {
        showCollectionsTable();
        lastFetchTime = millis(); // Record successful fetch time
      } else {
        Serial.println(F("Failed to fetch collections data"));
        // No error display to save space
      }

  } else {
      Serial.println(F("\nFailed to connect."));
  }

  lv_timer_handler();
}

// Fetch collections data from web server
bool fetchCollectionsData() {
  HTTPClient http;
  String url = "http://192.168.1.131:9090/collections.txt";
  
  Serial.printf("Fetching data from: %s\n", url.c_str());
  http.begin(url);
  
  // Set timeout
  http.setTimeout(10000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
    // Parse the data
    collectionCount = 0;
    int lineStart = 0;
    int lineEnd = 0;
    bool firstLine = true; // Skip header
    
    while (lineEnd >= 0 && collectionCount < MAX_ENTRIES) {
      lineEnd = payload.indexOf('\n', lineStart);
      String line;
      if (lineEnd >= 0) {
        line = payload.substring(lineStart, lineEnd);
        lineStart = lineEnd + 1;
      } else {
        line = payload.substring(lineStart);
      }
      
      line.trim();
      if (line.length() > 0) {
        if (firstLine) {
          // Skip header line
          firstLine = false;
        } else {
          // Parse date and collection type - handle multiple spaces
          line.replace('\t', ' '); // Replace tabs with spaces
          // Find first space
          int spaceIndex = line.indexOf(' ');
          if (spaceIndex > 0) {
            // Copy date
            String dateStr = line.substring(0, spaceIndex);
            snprintf(collections[collectionCount].date, sizeof(collections[collectionCount].date), "%s", dateStr.c_str());
            // Copy type
            String typeStr = line.substring(spaceIndex + 1);
            typeStr.trim();
            snprintf(collections[collectionCount].type, sizeof(collections[collectionCount].type), "%s", typeStr.c_str());
            collectionCount++;
          }
        }
      }
      
      if (lineEnd < 0) break;
    }
    
    http.end();
    Serial.printf("Parsed %d collection entries\n", collectionCount);
    return true;
  } else {
    Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return false;
  }
}

// Display collections data as a table
void showCollectionsTable() {
  lv_obj_clean(lv_screen_active());
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
  
  // Header "Next collections:"
  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text(title, LV_SYMBOL_TRASH " Next collections:");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
  
  // Horizontal line under title (gray)
  lv_obj_t* line = lv_obj_create(lv_screen_active());
  lv_obj_set_size(line, DISP_W - 20, 1);
  lv_obj_set_pos(line, 10, 35); // below title
  lv_obj_set_style_bg_color(line, lv_color_hex(0x808080), LV_PART_MAIN);
  lv_obj_set_style_border_width(line, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(line, 0, LV_PART_MAIN);
  
  // Column headers (shifted down)
  lv_obj_t* header_date = lv_label_create(lv_screen_active());
  lv_label_set_text(header_date, "Date");
  lv_obj_set_style_text_color(header_date, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(header_date, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_align(header_date, LV_ALIGN_TOP_LEFT, 10, 50);
  
  lv_obj_t* header_type = lv_label_create(lv_screen_active());
  lv_label_set_text(header_type, "Collection Type");
  lv_obj_set_style_text_color(header_type, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(header_type, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_align(header_type, LV_ALIGN_TOP_RIGHT, -10, 50);
  
  // Display data rows
  int y_offset = 80;
  for (int i = 0; i < collectionCount && i < 8; i++) { // Limit to 8 rows for display
    // Date column
    lv_obj_t* date_label = lv_label_create(lv_screen_active());
    lv_label_set_text(date_label, collections[i].date);
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(date_label, LV_ALIGN_TOP_LEFT, 10, y_offset);
    
    // Collection type column
    lv_obj_t* type_label = lv_label_create(lv_screen_active());
    lv_label_set_text(type_label, collections[i].type);
    // Choose color based on collection type
    lv_color_t type_color = lv_color_hex(0xFFFFFF); // default white
    if (strstr(collections[i].type, "Food Waste") != NULL) {
      type_color = lv_color_hex(0xFF0000); // red
    } else if (strstr(collections[i].type, "Garden Waste") != NULL) {
      type_color = lv_color_hex(0x90EE90); // light green
    } else if (strstr(collections[i].type, "Recycling") != NULL) {
      type_color = lv_color_hex(0x0000FF); // blue
    }
    lv_obj_set_style_text_color(type_label, type_color, LV_PART_MAIN);
    lv_obj_set_style_text_font(type_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(type_label, LV_ALIGN_TOP_RIGHT, -10, y_offset);
    
    y_offset += 25;
  }
}

// Attempt to refresh data every 12 hours
void attemptRefresh() {
  static unsigned long lastCheck = 0;
  // Check every 30 seconds to avoid excessive calls
  if (millis() - lastCheck < 30000) return;
  lastCheck = millis();

  if (millis() - lastFetchTime >= REFRESH_INTERVAL) {
    Serial.println(F("12‑hour refresh interval reached."));
    
    // Ensure Wi‑Fi is connected
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("Wi‑Fi disconnected, attempting reconnect..."));
      WiFi.reconnect();
      delay(5000); // Wait for connection
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("Reconnect failed, will try again later."));
        return;
      }
      Serial.println(F("Wi‑Fi reconnected."));
    }
    
    // Fetch new data
    if (fetchCollectionsData()) {
      showCollectionsTable();
      lastFetchTime = millis();
      Serial.println(F("Data refreshed successfully."));
    } else {
      Serial.println(F("Refresh fetch failed, keeping old data."));
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);

  tft.init();
  tft.setRotation(1);

  lv_init();
  static lv_color_t buf[240 * 10];
  auto disp = lv_display_create(DISP_W, DISP_H);
  lv_display_set_flush_cb(disp, lv_flush_cb);
  lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Show connecting screen
  lv_obj_clean(lv_screen_active());
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
  status_label = lv_label_create(lv_screen_active());
  lv_label_set_text(status_label, "Connecting to Wi-Fi...");
  lv_obj_set_style_text_color(status_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(status_label, DISP_W - 20);
  lv_obj_center(status_label);
  lv_timer_handler();
  delay(100);

  prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", "skynet");
    String pass = prefs.getString("pass", "ukstokes.com");
    prefs.end();

    start_wifi_connection(ssid.c_str(), pass.c_str());
}

void loop() {
  if (millis() - lastLvTick >= LVGL_TICK_PERIOD) {
    lv_tick_inc(LVGL_TICK_PERIOD);
    lv_timer_handler();
    lastLvTick = millis();
  }
  attemptRefresh();
  delay(5);
}
