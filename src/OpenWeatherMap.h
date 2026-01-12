/**
 * @file OpenWeatherMap.h
 * @brief OpenWeatherMap API Library for Arduino
 * @author Your Name
 * @version 1.0.0
 * @date 2026-01-08
 * 
 * This library provides easy access to OpenWeatherMap APIs including:
 * - Geocoding API (direct and reverse)
 * - Current Weather Data API
 * - Air Pollution API (current, forecast, historical)
 * - 5 Day / 3 Hour Forecast API
 * 
 * Supports:
 * - Arduino UNO R4 WiFi
 * - ESP32 series
 */

#ifndef OPENWEATHERMAP_H
#define OPENWEATHERMAP_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Platform-specific includes
#if defined(ARDUINO_UNOWIFIR4)
    #include <WiFiS3.h>
#elif defined(ESP32)
    #include <WiFi.h>
    #include <HTTPClient.h>
#else
    #error "Unsupported board! This library supports Arduino UNO R4 WiFi and ESP32 series."
#endif

// API Configuration
#define OWM_API_HOST "api.openweathermap.org"
#define OWM_GEO_HOST "api.openweathermap.org"
#define OWM_API_PORT_HTTP 80
#define OWM_API_PORT_HTTPS 443

// Cache settings
#define OWM_CACHE_DURATION_MS 60000  // Default cache duration: 60 seconds

// Buffer sizes
#define OWM_CITY_NAME_SIZE 64
#define OWM_COUNTRY_SIZE 8
#define OWM_DESCRIPTION_SIZE 64
#define OWM_ICON_SIZE 8
#define OWM_MAX_FORECAST_ITEMS 40
#define OWM_MAX_GEO_RESULTS 5

// Units of measurement
enum OWM_Units {
    OWM_UNITS_STANDARD,   // Kelvin, meter/sec
    OWM_UNITS_METRIC,     // Celsius, meter/sec
    OWM_UNITS_IMPERIAL    // Fahrenheit, miles/hour
};

// Air Quality Index levels
enum OWM_AQI {
    OWM_AQI_GOOD = 1,
    OWM_AQI_FAIR = 2,
    OWM_AQI_MODERATE = 3,
    OWM_AQI_POOR = 4,
    OWM_AQI_VERY_POOR = 5
};

// ============================================================================
// Data Structures
// ============================================================================

/**
 * @brief Geographic location data
 */
struct OWM_GeoLocation {
    char name[OWM_CITY_NAME_SIZE];
    char country[OWM_COUNTRY_SIZE];
    char state[32];
    float lat;
    float lon;
};

/**
 * @brief Weather condition data
 */
struct OWM_WeatherCondition {
    int id;                           // Weather condition id
    char main[32];                    // Group (Rain, Snow, Clouds etc.)
    char description[OWM_DESCRIPTION_SIZE];  // Detailed description
    char icon[OWM_ICON_SIZE];         // Weather icon id
};

/**
 * @brief Main weather parameters
 */
struct OWM_MainData {
    float temp;           // Temperature
    float feels_like;     // Feels like temperature
    float temp_min;       // Minimum temperature
    float temp_max;       // Maximum temperature
    int pressure;         // Atmospheric pressure (hPa)
    int humidity;         // Humidity (%)
    int sea_level;        // Sea level pressure (hPa)
    int grnd_level;       // Ground level pressure (hPa)
};

/**
 * @brief Wind data
 */
struct OWM_WindData {
    float speed;          // Wind speed
    int deg;              // Wind direction (degrees)
    float gust;           // Wind gust
};

/**
 * @brief Current weather data
 */
struct OWM_CurrentWeather {
    float lat;
    float lon;
    OWM_WeatherCondition weather;
    OWM_MainData main;
    int visibility;       // Visibility (meters)
    OWM_WindData wind;
    int clouds;           // Cloudiness (%)
    float rain_1h;        // Rain volume for last 1 hour (mm)
    float snow_1h;        // Snow volume for last 1 hour (mm)
    unsigned long dt;     // Time of data calculation (unix, UTC)
    char country[OWM_COUNTRY_SIZE];
    unsigned long sunrise;
    unsigned long sunset;
    int timezone;         // Shift from UTC (seconds)
    char name[OWM_CITY_NAME_SIZE];
};

/**
 * @brief Air pollution components
 */
struct OWM_AirComponents {
    float co;             // Carbon monoxide (μg/m³)
    float no;             // Nitrogen monoxide (μg/m³)
    float no2;            // Nitrogen dioxide (μg/m³)
    float o3;             // Ozone (μg/m³)
    float so2;            // Sulphur dioxide (μg/m³)
    float pm2_5;          // Fine particles (μg/m³)
    float pm10;           // Coarse particles (μg/m³)
    float nh3;            // Ammonia (μg/m³)
};

/**
 * @brief Air pollution data
 */
struct OWM_AirPollution {
    unsigned long dt;     // Date and time (unix, UTC)
    int aqi;              // Air Quality Index (1-5)
    OWM_AirComponents components;
};

/**
 * @brief Single forecast item (3-hour interval)
 */
struct OWM_ForecastItem {
    unsigned long dt;     // Time of data forecasted (unix, UTC)
    OWM_MainData main;
    OWM_WeatherCondition weather;
    OWM_WindData wind;
    int clouds;           // Cloudiness (%)
    int visibility;       // Visibility (meters)
    float pop;            // Probability of precipitation (0-1)
    float rain_3h;        // Rain volume for 3 hours (mm)
    float snow_3h;        // Snow volume for 3 hours (mm)
    char dt_txt[20];      // Time of data forecasted (ISO)
};

/**
 * @brief 5-day forecast data
 */
struct OWM_Forecast {
    int cnt;              // Number of timestamps
    OWM_ForecastItem items[OWM_MAX_FORECAST_ITEMS];
    char city_name[OWM_CITY_NAME_SIZE];
    char country[OWM_COUNTRY_SIZE];
    float lat;
    float lon;
    int timezone;
    unsigned long sunrise;
    unsigned long sunset;
};

// ============================================================================
// OpenWeatherMap Class
// ============================================================================

class OpenWeatherMap {
public:
    /**
     * @brief Construct a new OpenWeatherMap object
     */
    OpenWeatherMap();
    
    /**
     * @brief Initialize the library with API key
     * @param apiKey Your OpenWeatherMap API key
     * @param useHttps Set to true for HTTPS, false for HTTP (default, faster)
     */
    void begin(const char* apiKey, bool useHttps = false);
    
    /**
     * @brief Set the unit system for measurements
     * @param units OWM_UNITS_STANDARD, OWM_UNITS_METRIC, or OWM_UNITS_IMPERIAL
     */
    void setUnits(OWM_Units units);
    
    /**
     * @brief Set the language for descriptions
     * @param lang Language code (e.g., "en", "zh_cn", "de")
     */
    void setLanguage(const char* lang);
    
    /**
     * @brief Enable/disable debug output
     * @param enable True to enable debug output
     */
    void setDebug(bool enable);
    
    /**
     * @brief Set cache duration for weather data
     * @param durationMs Cache duration in milliseconds (0 to disable caching)
     */
    void setCacheDuration(unsigned long durationMs);
    
    // ========================================================================
    // Geocoding API
    // ========================================================================
    
    /**
     * @brief Get coordinates by location name (Direct Geocoding)
     * @param cityName City name
     * @param countryCode ISO 3166 country code (optional, can be NULL)
     * @param stateCode State code for US locations (optional, can be NULL)
     * @param results Array to store results
     * @param maxResults Maximum number of results (up to 5)
     * @return Number of results found, or -1 on error
     */
    int getCoordinatesByName(const char* cityName, const char* countryCode, 
                             const char* stateCode, OWM_GeoLocation* results, 
                             int maxResults = 5);
    
    /**
     * @brief Get coordinates by zip/postal code
     * @param zipCode Zip/postal code
     * @param countryCode ISO 3166 country code
     * @param location Pointer to store result
     * @return true on success, false on error
     */
    bool getCoordinatesByZip(const char* zipCode, const char* countryCode, 
                             OWM_GeoLocation* location);
    
    /**
     * @brief Get location name by coordinates (Reverse Geocoding)
     * @param lat Latitude
     * @param lon Longitude
     * @param results Array to store results
     * @param maxResults Maximum number of results
     * @return Number of results found, or -1 on error
     */
    int getLocationByCoordinates(float lat, float lon, OWM_GeoLocation* results, 
                                 int maxResults = 5);
    
    // ========================================================================
    // Current Weather API
    // ========================================================================
    
    /**
     * @brief Get current weather by coordinates
     * @param lat Latitude
     * @param lon Longitude
     * @param weather Pointer to store weather data
     * @return true on success, false on error
     */
    bool getCurrentWeather(float lat, float lon, OWM_CurrentWeather* weather);
    
    /**
     * @brief Get current weather by city name (uses geocoding internally)
     * @param cityName City name
     * @param countryCode ISO 3166 country code (optional)
     * @param weather Pointer to store weather data
     * @return true on success, false on error
     */
    bool getCurrentWeatherByCity(const char* cityName, const char* countryCode, 
                                 OWM_CurrentWeather* weather);
    
    // ========================================================================
    // Air Pollution API
    // ========================================================================
    
    /**
     * @brief Get current air pollution data
     * @param lat Latitude
     * @param lon Longitude
     * @param pollution Pointer to store air pollution data
     * @return true on success, false on error
     */
    bool getAirPollution(float lat, float lon, OWM_AirPollution* pollution);
    
    /**
     * @brief Get air pollution forecast (up to 4 days)
     * @param lat Latitude
     * @param lon Longitude
     * @param forecast Array to store forecast data
     * @param maxItems Maximum items to retrieve
     * @return Number of forecast items, or -1 on error
     */
    int getAirPollutionForecast(float lat, float lon, OWM_AirPollution* forecast, 
                                int maxItems);
    
    /**
     * @brief Get historical air pollution data
     * @param lat Latitude
     * @param lon Longitude
     * @param startTime Start time (Unix timestamp, UTC)
     * @param endTime End time (Unix timestamp, UTC)
     * @param history Array to store historical data
     * @param maxItems Maximum items to retrieve
     * @return Number of items, or -1 on error
     */
    int getAirPollutionHistory(float lat, float lon, unsigned long startTime, 
                               unsigned long endTime, OWM_AirPollution* history, 
                               int maxItems);
    
    // ========================================================================
    // 5-Day / 3-Hour Forecast API
    // ========================================================================
    
    /**
     * @brief Get 5-day weather forecast
     * @param lat Latitude
     * @param lon Longitude
     * @param forecast Pointer to store forecast data
     * @param cnt Number of timestamps to retrieve (optional, 0 for all)
     * @return true on success, false on error
     */
    bool getForecast(float lat, float lon, OWM_Forecast* forecast, int cnt = 0);
    
    /**
     * @brief Get 5-day weather forecast by city name
     * @param cityName City name
     * @param countryCode ISO 3166 country code (optional)
     * @param forecast Pointer to store forecast data
     * @param cnt Number of timestamps to retrieve (optional, 0 for all)
     * @return true on success, false on error
     */
    bool getForecastByCity(const char* cityName, const char* countryCode, 
                           OWM_Forecast* forecast, int cnt = 0);
    
    // ========================================================================
    // Utility Functions
    // ========================================================================
    
    /**
     * @brief Get AQI description string
     * @param aqi Air Quality Index value (1-5)
     * @return Description string
     */
    static const char* getAQIDescription(int aqi);
    
    /**
     * @brief Get weather icon URL
     * @param iconCode Icon code from weather data
     * @param buffer Buffer to store URL
     * @param bufferSize Size of buffer
     * @return Pointer to buffer
     */
    static char* getIconURL(const char* iconCode, char* buffer, size_t bufferSize);
    
    /**
     * @brief Get last HTTP response code
     * @return HTTP response code
     */
    int getLastHttpCode() const;
    
    /**
     * @brief Get last error message
     * @return Error message string
     */
    const char* getLastError() const;

private:
    char _apiKey[48];
    OWM_Units _units;
    char _lang[8];
    bool _debug;
    bool _useHttps;
    int _lastHttpCode;
    char _lastError[64];
    
    // Cache variables
    unsigned long _cacheDuration;
    unsigned long _lastWeatherTime;
    unsigned long _lastForecastTime;
    unsigned long _lastAirPollutionTime;
    float _cachedLat;
    float _cachedLon;
    OWM_CurrentWeather _cachedWeather;
    bool _hasCachedWeather;
    
    // HTTP methods
    bool httpGet(const char* host, const char* path, String& response);
    bool parseHttpResponse(String& response);
    
    // URL building helpers
    void buildUnitsParam(char* buffer, size_t size);
    void buildLangParam(char* buffer, size_t size);
    
    // JSON parsing helpers
    bool parseCurrentWeather(const String& json, OWM_CurrentWeather* weather);
    bool parseForecast(const String& json, OWM_Forecast* forecast);
    bool parseAirPollution(const String& json, OWM_AirPollution* pollution);
    int parseAirPollutionList(const String& json, OWM_AirPollution* list, int maxItems);
    int parseGeoLocations(const String& json, OWM_GeoLocation* locations, int maxResults);
    bool parseGeoZip(const String& json, OWM_GeoLocation* location);
    
    void parseWeatherCondition(JsonObject& obj, OWM_WeatherCondition* condition);
    void parseMainData(JsonObject& obj, OWM_MainData* main);
    void parseWindData(JsonObject& obj, OWM_WindData* wind);
    void parseAirComponents(JsonObject& obj, OWM_AirComponents* components);
    
    void debugPrint(const char* message);
    void debugPrintln(const char* message);
    void setError(const char* error);
};

#endif // OPENWEATHERMAP_H
