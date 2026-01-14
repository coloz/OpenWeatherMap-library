/**
 * @file AirPollution.ino
 * @brief Example: Get air pollution data
 * 
 * This example demonstrates how to get current air pollution,
 * forecast, and historical data using the OpenWeatherMap library.
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
const float LATITUDE = 39.9042f;   // Beijing
const float LONGITUDE = 116.4074f;

OpenWeatherMap weather;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    
    Serial.println();
    Serial.println("OpenWeatherMap - Air Pollution Example");
    Serial.println("=======================================");
    
    connectWiFi();
    
    // Initialize
    weather.begin(OWM_API_KEY);
    weather.setTimeout(5000);  // 5 second timeout
    weather.setDebug(true);
    
    // Get current air pollution
    Serial.println("\n--- Current Air Pollution ---");
    getCurrentAirPollution();
    
    delay(2000);
    
    // Get air pollution forecast
    Serial.println("\n--- Air Pollution Forecast (next 24h) ---");
    getAirPollutionForecast();
}

void loop() {
    // Update every 30 minutes
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1800000) {
        getCurrentAirPollution();
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

void getCurrentAirPollution() {
    OWM_AirPollution data;
    
    if (weather.getAirPollution(LATITUDE, LONGITUDE, &data)) {
        printAirPollution(&data);
    } else {
        Serial.print("Error: ");
        Serial.println(weather.getLastError());
    }
}

void getAirPollutionForecast() {
    // Get 8 forecast items (24 hours, each 3 hours apart)
    OWM_AirPollution forecast[8];
    
    int count = weather.getAirPollutionForecast(LATITUDE, LONGITUDE, forecast, 8);
    
    if (count > 0) {
        Serial.print("Retrieved ");
        Serial.print(count);
        Serial.println(" forecast items\n");
        
        for (int i = 0; i < count; i++) {
            Serial.print("[");
            Serial.print(i + 1);
            Serial.print("] ");
            printAirPollution(&forecast[i]);
        }
    } else {
        Serial.print("Error: ");
        Serial.println(weather.getLastError());
    }
}

void printAirPollution(OWM_AirPollution* data) {
    Serial.println("----- Air Quality Data -----");
    
    Serial.print("Timestamp: ");
    Serial.println(data->dt);
    
    Serial.print("Air Quality Index: ");
    Serial.print(data->aqi);
    Serial.print(" - ");
    Serial.println(OpenWeatherMap::getAQIDescription(data->aqi));
    
    // Print AQI indicator
    Serial.print("AQI Level: ");
    for (int i = 0; i < data->aqi; i++) {
        Serial.print("*");
    }
    Serial.println();
    
    Serial.println("\nPollutant Concentrations (μg/m³):");
    
    Serial.print("  CO  (Carbon monoxide):   ");
    Serial.println(data->components.co, 2);
    
    Serial.print("  NO  (Nitrogen monoxide): ");
    Serial.println(data->components.no, 4);
    
    Serial.print("  NO2 (Nitrogen dioxide):  ");
    Serial.println(data->components.no2, 2);
    
    Serial.print("  O3  (Ozone):             ");
    Serial.println(data->components.o3, 2);
    
    Serial.print("  SO2 (Sulphur dioxide):   ");
    Serial.println(data->components.so2, 2);
    
    Serial.print("  PM2.5 (Fine particles):  ");
    Serial.println(data->components.pm2_5, 2);
    
    Serial.print("  PM10 (Coarse particles): ");
    Serial.println(data->components.pm10, 2);
    
    Serial.print("  NH3 (Ammonia):           ");
    Serial.println(data->components.nh3, 2);
    
    // Simple health recommendation based on AQI
    Serial.print("\nHealth Advisory: ");
    switch (data->aqi) {
        case OWM_AQI_GOOD:
            Serial.println("Air quality is good. Enjoy outdoor activities!");
            break;
        case OWM_AQI_FAIR:
            Serial.println("Air quality is acceptable. Unusually sensitive people should reduce outdoor activity.");
            break;
        case OWM_AQI_MODERATE:
            Serial.println("Sensitive groups may experience health effects. Consider reducing prolonged outdoor exertion.");
            break;
        case OWM_AQI_POOR:
            Serial.println("Health effects possible for everyone. Limit outdoor exertion.");
            break;
        case OWM_AQI_VERY_POOR:
            Serial.println("Health warnings! Avoid outdoor activities.");
            break;
    }
    
    Serial.println("----------------------------\n");
}
