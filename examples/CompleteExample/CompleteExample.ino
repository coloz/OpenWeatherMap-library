/**
 * @file CompleteExample.ino
 * @brief Complete example using all OpenWeatherMap APIs
 * 
 * This example demonstrates a complete weather station application
 * that uses all available APIs:
 * - Geocoding API
 * - Current Weather API
 * - Air Pollution API
 * - 5-Day Forecast API
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

// Default location (can be changed via Serial)
String cityName = "Shanghai";
String countryCode = "CN";
float latitude = 31.2304f;
float longitude = 121.4737f;
bool useCoordinates = false;

OpenWeatherMap weather;

// Update intervals (in milliseconds)
const unsigned long WEATHER_UPDATE_INTERVAL = 600000;  // 10 minutes
const unsigned long AQI_UPDATE_INTERVAL = 900000;       // 15 minutes
const unsigned long FORECAST_UPDATE_INTERVAL = 3600000; // 1 hour

unsigned long lastWeatherUpdate = 0;
unsigned long lastAQIUpdate = 0;
unsigned long lastForecastUpdate = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    
    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║   OpenWeatherMap Complete Weather App    ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();
    
    connectWiFi();
    
    // Initialize weather library
    weather.begin(OWM_API_KEY);
    weather.setUnits(OWM_UNITS_METRIC);
    weather.setLanguage("en");
    weather.setTimeout(5000);  // 5 second timeout
    weather.setDebug(false);  // Set to true for debugging
    
    Serial.println("\nCommands:");
    Serial.println("  'w' - Get current weather");
    Serial.println("  'f' - Get 5-day forecast");
    Serial.println("  'a' - Get air pollution");
    Serial.println("  'l' - Change location");
    Serial.println("  'h' - Show this help");
    Serial.println();
    
    // Initial data fetch
    updateAllData();
}

void loop() {
    // Handle Serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        handleCommand(cmd);
    }
    
    // Auto-update based on intervals
    unsigned long now = millis();
    
    if (now - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
        Serial.println("\n[Auto-update] Current weather...");
        showCurrentWeather();
        lastWeatherUpdate = now;
    }
    
    if (now - lastAQIUpdate >= AQI_UPDATE_INTERVAL) {
        Serial.println("\n[Auto-update] Air quality...");
        showAirPollution();
        lastAQIUpdate = now;
    }
    
    if (now - lastForecastUpdate >= FORECAST_UPDATE_INTERVAL) {
        Serial.println("\n[Auto-update] Forecast...");
        showForecast();
        lastForecastUpdate = now;
    }
}

void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(" Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println(" Failed!");
        Serial.println("Please check WiFi credentials and restart.");
        while (1) delay(1000);
    }
}

void handleCommand(char cmd) {
    switch (cmd) {
        case 'w':
        case 'W':
            showCurrentWeather();
            break;
        case 'f':
        case 'F':
            showForecast();
            break;
        case 'a':
        case 'A':
            showAirPollution();
            break;
        case 'l':
        case 'L':
            changeLocation();
            break;
        case 'h':
        case 'H':
            showHelp();
            break;
        case '\n':
        case '\r':
            // Ignore newline characters
            break;
        default:
            if (cmd >= 32 && cmd <= 126) {  // Printable characters only
                Serial.print("Unknown command: ");
                Serial.println(cmd);
                Serial.println("Press 'h' for help");
            }
            break;
    }
}

void updateAllData() {
    Serial.println("\n=== Fetching all weather data ===\n");
    
    // First, resolve coordinates if using city name
    if (!useCoordinates) {
        resolveLocation();
    }
    
    showCurrentWeather();
    delay(500);
    showAirPollution();
    delay(500);
    showForecastSummary();
    
    lastWeatherUpdate = millis();
    lastAQIUpdate = millis();
    lastForecastUpdate = millis();
}

void resolveLocation() {
    OWM_GeoLocation loc;
    int count = weather.getCoordinatesByName(cityName.c_str(), countryCode.c_str(), NULL, &loc, 1);
    
    if (count > 0) {
        latitude = loc.lat;
        longitude = loc.lon;
        Serial.print("Resolved location: ");
        Serial.print(loc.name);
        Serial.print(" (");
        Serial.print(latitude, 2);
        Serial.print(", ");
        Serial.print(longitude, 2);
        Serial.println(")");
    }
}

void showCurrentWeather() {
    Serial.println("\n┌─────────────────────────────────────┐");
    Serial.println("│         CURRENT WEATHER             │");
    Serial.println("└─────────────────────────────────────┘");
    
    OWM_CurrentWeather data;
    bool success = useCoordinates ? 
                   weather.getCurrentWeather(latitude, longitude, &data) :
                   weather.getCurrentWeatherByCity(cityName.c_str(), countryCode.c_str(), &data);
    
    if (success) {
        Serial.print("📍 ");
        Serial.print(data.name);
        Serial.print(", ");
        Serial.println(data.country);
        
        Serial.print("🌡️  Temperature: ");
        Serial.print(data.main.temp, 1);
        Serial.print("°C (feels like ");
        Serial.print(data.main.feels_like, 1);
        Serial.println("°C)");
        
        Serial.print("☁️  Conditions: ");
        Serial.print(data.weather.main);
        Serial.print(" - ");
        Serial.println(data.weather.description);
        
        Serial.print("💧 Humidity: ");
        Serial.print(data.main.humidity);
        Serial.println("%");
        
        Serial.print("🌬️  Wind: ");
        Serial.print(data.wind.speed, 1);
        Serial.print(" m/s @ ");
        Serial.print(data.wind.deg);
        Serial.println("°");
        
        Serial.print("📊 Pressure: ");
        Serial.print(data.main.pressure);
        Serial.println(" hPa");
        
        Serial.print("👁️  Visibility: ");
        Serial.print(data.visibility / 1000.0, 1);
        Serial.println(" km");
    } else {
        Serial.print("❌ Error: ");
        Serial.println(weather.getLastError());
    }
}

void showAirPollution() {
    Serial.println("\n┌─────────────────────────────────────┐");
    Serial.println("│          AIR QUALITY                │");
    Serial.println("└─────────────────────────────────────┘");
    
    OWM_AirPollution data;
    
    if (weather.getAirPollution(latitude, longitude, &data)) {
        Serial.print("🌫️  Air Quality Index: ");
        Serial.print(data.aqi);
        Serial.print(" (");
        Serial.print(OpenWeatherMap::getAQIDescription(data.aqi));
        Serial.println(")");
        
        // Visual AQI bar
        Serial.print("   [");
        for (int i = 1; i <= 5; i++) {
            if (i <= data.aqi) {
                switch (i) {
                    case 1: Serial.print("▓"); break;
                    case 2: Serial.print("▓"); break;
                    case 3: Serial.print("▓"); break;
                    case 4: Serial.print("▓"); break;
                    case 5: Serial.print("▓"); break;
                }
            } else {
                Serial.print("░");
            }
        }
        Serial.println("]");
        
        Serial.println("\n   Key pollutants (μg/m³):");
        Serial.print("   PM2.5: ");
        Serial.print(data.components.pm2_5, 1);
        Serial.print("  |  PM10: ");
        Serial.print(data.components.pm10, 1);
        Serial.print("  |  O3: ");
        Serial.println(data.components.o3, 1);
    } else {
        Serial.print("❌ Error: ");
        Serial.println(weather.getLastError());
    }
}

void showForecast() {
    Serial.println("\n┌─────────────────────────────────────┐");
    Serial.println("│         5-DAY FORECAST              │");
    Serial.println("└─────────────────────────────────────┘");
    
    OWM_Forecast forecast;
    
    if (weather.getForecast(latitude, longitude, &forecast, 0)) {
        Serial.print("📅 Showing ");
        Serial.print(forecast.cnt);
        Serial.println(" forecast periods\n");
        
        for (int i = 0; i < forecast.cnt && i < 40; i++) {
            OWM_ForecastItem* item = &forecast.items[i];
            
            Serial.print(item->dt_txt);
            Serial.print(" | ");
            Serial.print(item->main.temp, 0);
            Serial.print("°C | ");
            Serial.print(item->weather.main);
            Serial.print(" | ");
            Serial.print(item->pop * 100, 0);
            Serial.println("% precip");
        }
    } else {
        Serial.print("❌ Error: ");
        Serial.println(weather.getLastError());
    }
}

void showForecastSummary() {
    Serial.println("\n┌─────────────────────────────────────┐");
    Serial.println("│       FORECAST SUMMARY (24h)        │");
    Serial.println("└─────────────────────────────────────┘");
    
    OWM_Forecast forecast;
    
    // Get 8 items (24 hours)
    if (weather.getForecast(latitude, longitude, &forecast, 8)) {
        float minTemp = 999, maxTemp = -999;
        float maxPop = 0;
        
        for (int i = 0; i < forecast.cnt; i++) {
            if (forecast.items[i].main.temp < minTemp) minTemp = forecast.items[i].main.temp;
            if (forecast.items[i].main.temp > maxTemp) maxTemp = forecast.items[i].main.temp;
            if (forecast.items[i].pop > maxPop) maxPop = forecast.items[i].pop;
        }
        
        Serial.print("🌡️  Next 24h: ");
        Serial.print(minTemp, 0);
        Serial.print("°C to ");
        Serial.print(maxTemp, 0);
        Serial.println("°C");
        
        Serial.print("🌧️  Max precipitation chance: ");
        Serial.print(maxPop * 100, 0);
        Serial.println("%");
    }
}

void changeLocation() {
    Serial.println("\nEnter new location (format: CityName or CityName,CountryCode):");
    
    // Wait for input
    while (!Serial.available()) {
        delay(100);
    }
    
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() > 0) {
        int commaIndex = input.indexOf(',');
        if (commaIndex > 0) {
            cityName = input.substring(0, commaIndex);
            countryCode = input.substring(commaIndex + 1);
            countryCode.trim();
        } else {
            cityName = input;
            countryCode = "";
        }
        
        useCoordinates = false;
        
        Serial.print("Location changed to: ");
        Serial.print(cityName);
        if (countryCode.length() > 0) {
            Serial.print(", ");
            Serial.print(countryCode);
        }
        Serial.println();
        
        // Fetch new data
        updateAllData();
    }
}

void showHelp() {
    Serial.println("\n┌─────────────────────────────────────┐");
    Serial.println("│              HELP                   │");
    Serial.println("└─────────────────────────────────────┘");
    Serial.println("Commands:");
    Serial.println("  'w' - Show current weather");
    Serial.println("  'f' - Show 5-day forecast");
    Serial.println("  'a' - Show air pollution");
    Serial.println("  'l' - Change location");
    Serial.println("  'h' - Show this help");
    Serial.println();
    Serial.print("Current location: ");
    Serial.print(cityName);
    if (countryCode.length() > 0) {
        Serial.print(", ");
        Serial.print(countryCode);
    }
    Serial.println();
    Serial.print("Coordinates: ");
    Serial.print(latitude, 4);
    Serial.print(", ");
    Serial.println(longitude, 4);
}
