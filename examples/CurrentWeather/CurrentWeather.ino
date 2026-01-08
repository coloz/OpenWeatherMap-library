/**
 * @file CurrentWeather.ino
 * @brief Example: Get current weather data
 * 
 * This example demonstrates how to get current weather data
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

// OpenWeatherMap API key (get it from https://openweathermap.org/api)
const char* OWM_API_KEY = "YOUR_API_KEY";

// Location settings
const char* CITY_NAME = "Shanghai";
const char* COUNTRY_CODE = "CN";

// Or use coordinates directly
const float LATITUDE = 31.2304f;
const float LONGITUDE = 121.4737f;

OpenWeatherMap weather;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    
    Serial.println();
    Serial.println("OpenWeatherMap - Current Weather Example");
    Serial.println("========================================");
    
    // Connect to WiFi
    connectWiFi();
    
    // Initialize the weather library
    weather.begin(OWM_API_KEY);
    weather.setUnits(OWM_UNITS_METRIC);    // Use Celsius
    weather.setLanguage("zh_cn");           // Chinese description
    weather.setDebug(true);                 // Enable debug output
    
    // Method 1: Get weather by coordinates
    Serial.println("\n--- Get Weather by Coordinates ---");
    getWeatherByCoordinates();
    
    delay(2000);
    
    // Method 2: Get weather by city name
    Serial.println("\n--- Get Weather by City Name ---");
    getWeatherByCityName();
}

void loop() {
    // Update weather every 10 minutes
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 600000) {
        getWeatherByCoordinates();
        lastUpdate = millis();
    }
}

void connectWiFi() {
    Serial.print("Connecting to WiFi");
    
#if defined(ESP32)
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#elif defined(ARDUINO_UNOWIFIR4)
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
}

void getWeatherByCoordinates() {
    OWM_CurrentWeather data;
    
    if (weather.getCurrentWeather(LATITUDE, LONGITUDE, &data)) {
        printWeatherData(&data);
    } else {
        Serial.print("Error: ");
        Serial.println(weather.getLastError());
    }
}

void getWeatherByCityName() {
    OWM_CurrentWeather data;
    
    if (weather.getCurrentWeatherByCity(CITY_NAME, COUNTRY_CODE, &data)) {
        printWeatherData(&data);
    } else {
        Serial.print("Error: ");
        Serial.println(weather.getLastError());
    }
}

void printWeatherData(OWM_CurrentWeather* data) {
    Serial.println("\n----- Current Weather -----");
    
    Serial.print("Location: ");
    Serial.print(data->name);
    Serial.print(", ");
    Serial.println(data->country);
    
    Serial.print("Coordinates: ");
    Serial.print(data->lat, 4);
    Serial.print(", ");
    Serial.println(data->lon, 4);
    
    Serial.print("Weather: ");
    Serial.print(data->weather.main);
    Serial.print(" - ");
    Serial.println(data->weather.description);
    
    Serial.print("Temperature: ");
    Serial.print(data->main.temp, 1);
    Serial.println(" °C");
    
    Serial.print("Feels Like: ");
    Serial.print(data->main.feels_like, 1);
    Serial.println(" °C");
    
    Serial.print("Min/Max: ");
    Serial.print(data->main.temp_min, 1);
    Serial.print(" / ");
    Serial.print(data->main.temp_max, 1);
    Serial.println(" °C");
    
    Serial.print("Humidity: ");
    Serial.print(data->main.humidity);
    Serial.println(" %");
    
    Serial.print("Pressure: ");
    Serial.print(data->main.pressure);
    Serial.println(" hPa");
    
    Serial.print("Wind: ");
    Serial.print(data->wind.speed, 1);
    Serial.print(" m/s, ");
    Serial.print(data->wind.deg);
    Serial.println("°");
    
    Serial.print("Clouds: ");
    Serial.print(data->clouds);
    Serial.println(" %");
    
    Serial.print("Visibility: ");
    Serial.print(data->visibility / 1000.0, 1);
    Serial.println(" km");
    
    if (data->rain_1h > 0) {
        Serial.print("Rain (1h): ");
        Serial.print(data->rain_1h, 2);
        Serial.println(" mm");
    }
    
    if (data->snow_1h > 0) {
        Serial.print("Snow (1h): ");
        Serial.print(data->snow_1h, 2);
        Serial.println(" mm");
    }
    
    // Get weather icon URL
    char iconUrl[64];
    OpenWeatherMap::getIconURL(data->weather.icon, iconUrl, sizeof(iconUrl));
    Serial.print("Icon URL: ");
    Serial.println(iconUrl);
    
    Serial.println("---------------------------\n");
}
