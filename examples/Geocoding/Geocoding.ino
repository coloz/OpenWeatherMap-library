/**
 * @file Geocoding.ino
 * @brief Example: Use Geocoding API
 * 
 * This example demonstrates how to use the Geocoding API
 * for location lookup and reverse geocoding.
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

OpenWeatherMap weather;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    
    Serial.println();
    Serial.println("OpenWeatherMap - Geocoding Example");
    Serial.println("===================================");
    
    connectWiFi();
    
    // Initialize
    weather.begin(OWM_API_KEY);
    weather.setTimeout(5000);  // 5 second timeout
    weather.setDebug(true);
    
    // Example 1: Get coordinates by city name
    Serial.println("\n--- Example 1: City Name to Coordinates ---");
    getCoordinatesByCityName("London", "GB", NULL);
    
    delay(1000);
    
    // Example 2: Search for cities with same name
    Serial.println("\n--- Example 2: Search Multiple Results ---");
    searchCityName("London", NULL, NULL, 5);
    
    delay(1000);
    
    // Example 3: US city with state code
    Serial.println("\n--- Example 3: US City with State ---");
    getCoordinatesByCityName("Portland", "US", "OR");
    
    delay(1000);
    
    // Example 4: Get coordinates by ZIP code
    Serial.println("\n--- Example 4: ZIP Code to Coordinates ---");
    getCoordinatesByZip("10001", "US");  // New York
    
    delay(1000);
    
    // Example 5: Reverse geocoding
    Serial.println("\n--- Example 5: Coordinates to Location ---");
    getLocationByCoordinates(35.6762f, 139.6503f);  // Tokyo
    
    delay(1000);
    
    // Example 6: Get weather after geocoding
    Serial.println("\n--- Example 6: Geocode then Get Weather ---");
    geocodeAndGetWeather("Paris", "FR");
}

void loop() {
    // Nothing to do in loop
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

void getCoordinatesByCityName(const char* city, const char* country, const char* state) {
    OWM_GeoLocation location;
    
    int count = weather.getCoordinatesByName(city, country, state, &location, 1);
    
    if (count > 0) {
        Serial.println("\nLocation found:");
        printLocation(&location);
    } else {
        Serial.print("Error: ");
        Serial.println(weather.getLastError());
    }
}

void searchCityName(const char* city, const char* country, const char* state, int maxResults) {
    OWM_GeoLocation locations[5];
    
    int count = weather.getCoordinatesByName(city, country, state, locations, maxResults);
    
    if (count > 0) {
        Serial.print("\nFound ");
        Serial.print(count);
        Serial.println(" location(s):\n");
        
        for (int i = 0; i < count; i++) {
            Serial.print("[");
            Serial.print(i + 1);
            Serial.println("]");
            printLocation(&locations[i]);
        }
    } else {
        Serial.print("Error: ");
        Serial.println(weather.getLastError());
    }
}

void getCoordinatesByZip(const char* zip, const char* country) {
    OWM_GeoLocation location;
    
    if (weather.getCoordinatesByZip(zip, country, &location)) {
        Serial.println("\nLocation found:");
        printLocation(&location);
    } else {
        Serial.print("Error: ");
        Serial.println(weather.getLastError());
    }
}

void getLocationByCoordinates(float lat, float lon) {
    OWM_GeoLocation locations[3];
    
    int count = weather.getLocationByCoordinates(lat, lon, locations, 3);
    
    if (count > 0) {
        Serial.print("\nFound ");
        Serial.print(count);
        Serial.print(" location(s) near (");
        Serial.print(lat, 4);
        Serial.print(", ");
        Serial.print(lon, 4);
        Serial.println("):\n");
        
        for (int i = 0; i < count; i++) {
            Serial.print("[");
            Serial.print(i + 1);
            Serial.println("]");
            printLocation(&locations[i]);
        }
    } else {
        Serial.print("Error: ");
        Serial.println(weather.getLastError());
    }
}

void geocodeAndGetWeather(const char* city, const char* country) {
    // Step 1: Geocode
    OWM_GeoLocation location;
    int count = weather.getCoordinatesByName(city, country, NULL, &location, 1);
    
    if (count <= 0) {
        Serial.print("Geocoding failed: ");
        Serial.println(weather.getLastError());
        return;
    }
    
    Serial.print("\nGeocoded '");
    Serial.print(city);
    Serial.print("' to: ");
    Serial.print(location.lat, 4);
    Serial.print(", ");
    Serial.println(location.lon, 4);
    
    // Step 2: Get weather using coordinates
    OWM_CurrentWeather weatherData;
    if (weather.getCurrentWeather(location.lat, location.lon, &weatherData)) {
        Serial.println("\nWeather data retrieved successfully!");
        Serial.print("Temperature in ");
        Serial.print(weatherData.name);
        Serial.print(": ");
        Serial.print(weatherData.main.temp, 1);
        Serial.println("°C");
        Serial.print("Conditions: ");
        Serial.println(weatherData.weather.description);
    } else {
        Serial.print("Weather fetch failed: ");
        Serial.println(weather.getLastError());
    }
}

void printLocation(OWM_GeoLocation* loc) {
    Serial.print("  Name: ");
    Serial.println(loc->name);
    
    if (strlen(loc->state) > 0) {
        Serial.print("  State: ");
        Serial.println(loc->state);
    }
    
    Serial.print("  Country: ");
    Serial.println(loc->country);
    
    Serial.print("  Coordinates: ");
    Serial.print(loc->lat, 4);
    Serial.print(", ");
    Serial.println(loc->lon, 4);
    Serial.println();
}
