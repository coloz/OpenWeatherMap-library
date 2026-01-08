/**
 * @file Forecast5Day.ino
 * @brief Example: Get 5-day / 3-hour weather forecast
 * 
 * This example demonstrates how to get 5-day weather forecast
 * using the OpenWeatherMap library.
 * 
 * Supported boards:
 * - Arduino UNO R4 WiFi
 * - ESP32 series
 */

#include <OpenWeatherMap.h>

// WiFi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// OpenWeatherMap API key
const char* OWM_API_KEY = "YOUR_API_KEY";

// Location
const float LATITUDE = 31.2304f;   // Shanghai
const float LONGITUDE = 121.4737f;

OpenWeatherMap weather;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    
    Serial.println();
    Serial.println("OpenWeatherMap - 5-Day Forecast Example");
    Serial.println("========================================");
    
    connectWiFi();
    
    // Initialize
    weather.begin(OWM_API_KEY);
    weather.setUnits(OWM_UNITS_METRIC);
    weather.setLanguage("en");
    weather.setDebug(true);
    
    // Get full forecast (all 40 timestamps = 5 days)
    Serial.println("\n--- Full 5-Day Forecast ---");
    getForecast(0);
    
    delay(3000);
    
    // Get limited forecast (8 timestamps = 24 hours)
    Serial.println("\n--- 24-Hour Forecast (8 items) ---");
    getForecast(8);
}

void loop() {
    // Update forecast every hour
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 3600000) {
        getForecast(8);
        lastUpdate = millis();
    }
}

void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
}

void getForecast(int count) {
    OWM_Forecast forecast;
    
    if (weather.getForecast(LATITUDE, LONGITUDE, &forecast, count)) {
        printForecast(&forecast);
    } else {
        Serial.print("Error: ");
        Serial.println(weather.getLastError());
    }
}

void printForecast(OWM_Forecast* data) {
    Serial.println("\n========== Weather Forecast ==========");
    
    Serial.print("City: ");
    Serial.print(data->city_name);
    Serial.print(", ");
    Serial.println(data->country);
    
    Serial.print("Coordinates: ");
    Serial.print(data->lat, 4);
    Serial.print(", ");
    Serial.println(data->lon, 4);
    
    Serial.print("Timezone: UTC");
    if (data->timezone >= 0) Serial.print("+");
    Serial.print(data->timezone / 3600);
    Serial.println(" hours");
    
    Serial.print("Total forecasts: ");
    Serial.println(data->cnt);
    
    Serial.println("\n----- Forecast Items -----");
    
    for (int i = 0; i < data->cnt; i++) {
        OWM_ForecastItem* item = &data->items[i];
        
        Serial.println();
        Serial.print("[");
        Serial.print(i + 1);
        Serial.print("] ");
        Serial.println(item->dt_txt);
        
        Serial.print("    Weather: ");
        Serial.print(item->weather.main);
        Serial.print(" - ");
        Serial.println(item->weather.description);
        
        Serial.print("    Temp: ");
        Serial.print(item->main.temp, 1);
        Serial.print("°C (feels ");
        Serial.print(item->main.feels_like, 1);
        Serial.println("°C)");
        
        Serial.print("    Min/Max: ");
        Serial.print(item->main.temp_min, 1);
        Serial.print(" / ");
        Serial.print(item->main.temp_max, 1);
        Serial.println("°C");
        
        Serial.print("    Humidity: ");
        Serial.print(item->main.humidity);
        Serial.print("%, Pressure: ");
        Serial.print(item->main.pressure);
        Serial.println(" hPa");
        
        Serial.print("    Wind: ");
        Serial.print(item->wind.speed, 1);
        Serial.print(" m/s @ ");
        Serial.print(item->wind.deg);
        Serial.println("°");
        
        Serial.print("    Clouds: ");
        Serial.print(item->clouds);
        Serial.print("%, Visibility: ");
        Serial.print(item->visibility / 1000.0, 1);
        Serial.println(" km");
        
        Serial.print("    Precipitation Prob: ");
        Serial.print(item->pop * 100, 0);
        Serial.println("%");
        
        if (item->rain_3h > 0) {
            Serial.print("    Rain (3h): ");
            Serial.print(item->rain_3h, 2);
            Serial.println(" mm");
        }
        
        if (item->snow_3h > 0) {
            Serial.print("    Snow (3h): ");
            Serial.print(item->snow_3h, 2);
            Serial.println(" mm");
        }
    }
    
    Serial.println("\n=======================================\n");
}
