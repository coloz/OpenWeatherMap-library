/**
 * @file OpenWeatherMap.cpp
 * @brief OpenWeatherMap API Library Implementation
 */

#include "OpenWeatherMap.h"

// ============================================================================
// Constructor & Initialization
// ============================================================================

OpenWeatherMap::OpenWeatherMap() {
    _apiKey[0] = '\0';
    _units = OWM_UNITS_METRIC;
    strcpy(_lang, "en");
    _debug = false;
    _useHttps = false;
    _lastHttpCode = 0;
    _lastError[0] = '\0';
    _timeout = OWM_DEFAULT_TIMEOUT_MS;
    
    // Cache initialization
    _cacheDuration = OWM_CACHE_DURATION_MS;
    _lastWeatherTime = 0;
    _lastForecastTime = 0;
    _lastAirPollutionTime = 0;
    _cachedLat = 0;
    _cachedLon = 0;
    _hasCachedWeather = false;
}

void OpenWeatherMap::begin(const char* apiKey, bool useHttps) {
    strncpy(_apiKey, apiKey, sizeof(_apiKey) - 1);
    _apiKey[sizeof(_apiKey) - 1] = '\0';
    _useHttps = useHttps;
}

void OpenWeatherMap::setUnits(OWM_Units units) {
    _units = units;
}

void OpenWeatherMap::setLanguage(const char* lang) {
    strncpy(_lang, lang, sizeof(_lang) - 1);
    _lang[sizeof(_lang) - 1] = '\0';
}

void OpenWeatherMap::setDebug(bool enable) {
    _debug = enable;
}

void OpenWeatherMap::setCacheDuration(unsigned long durationMs) {
    _cacheDuration = durationMs;
}

void OpenWeatherMap::setTimeout(unsigned long timeoutMs) {
    _timeout = timeoutMs;
}

// ============================================================================
// Geocoding API Implementation
// ============================================================================

int OpenWeatherMap::getCoordinatesByName(const char* cityName, const char* countryCode, 
                                          const char* stateCode, OWM_GeoLocation* results, 
                                          int maxResults) {
    if (maxResults > OWM_MAX_GEO_RESULTS) {
        maxResults = OWM_MAX_GEO_RESULTS;
    }
    
    // Build query string
    String query = cityName;
    if (stateCode != NULL && strlen(stateCode) > 0) {
        query += ",";
        query += stateCode;
    }
    if (countryCode != NULL && strlen(countryCode) > 0) {
        query += ",";
        query += countryCode;
    }
    
    // URL encode the query
    String encodedQuery = "";
    for (size_t i = 0; i < query.length(); i++) {
        char c = query[i];
        if (c == ' ') {
            encodedQuery += "%20";
        } else if (c == ',') {
            encodedQuery += ",";
        } else {
            encodedQuery += c;
        }
    }
    
    // Build path
    char path[256];
    snprintf(path, sizeof(path), 
             "/geo/1.0/direct?q=%s&limit=%d&appid=%s",
             encodedQuery.c_str(), maxResults, _apiKey);
    
    String response;
    if (!httpGet(OWM_GEO_HOST, path, response)) {
        return -1;
    }
    
    return parseGeoLocations(response, results, maxResults);
}

bool OpenWeatherMap::getCoordinatesByZip(const char* zipCode, const char* countryCode, 
                                          OWM_GeoLocation* location) {
    char path[256];
    snprintf(path, sizeof(path), 
             "/geo/1.0/zip?zip=%s,%s&appid=%s",
             zipCode, countryCode, _apiKey);
    
    String response;
    if (!httpGet(OWM_GEO_HOST, path, response)) {
        return false;
    }
    
    return parseGeoZip(response, location);
}

int OpenWeatherMap::getLocationByCoordinates(float lat, float lon, 
                                              OWM_GeoLocation* results, int maxResults) {
    if (maxResults > OWM_MAX_GEO_RESULTS) {
        maxResults = OWM_MAX_GEO_RESULTS;
    }
    
    char path[256];
    snprintf(path, sizeof(path), 
             "/geo/1.0/reverse?lat=%.4f&lon=%.4f&limit=%d&appid=%s",
             lat, lon, maxResults, _apiKey);
    
    String response;
    if (!httpGet(OWM_GEO_HOST, path, response)) {
        return -1;
    }
    
    return parseGeoLocations(response, results, maxResults);
}

// ============================================================================
// Current Weather API Implementation
// ============================================================================

bool OpenWeatherMap::getCurrentWeather(float lat, float lon, OWM_CurrentWeather* weather) {
    // Check cache first
    if (_cacheDuration > 0 && _hasCachedWeather) {
        unsigned long now = millis();
        // Check if cache is still valid and coordinates match
        if ((now - _lastWeatherTime) < _cacheDuration &&
            abs(_cachedLat - lat) < 0.01 && abs(_cachedLon - lon) < 0.01) {
            debugPrintln("Using cached weather data");
            memcpy(weather, &_cachedWeather, sizeof(OWM_CurrentWeather));
            return true;
        }
    }
    
    char unitsParam[16], langParam[16];
    buildUnitsParam(unitsParam, sizeof(unitsParam));
    buildLangParam(langParam, sizeof(langParam));
    
    char path[256];
    snprintf(path, sizeof(path), 
             "/data/2.5/weather?lat=%.4f&lon=%.4f%s%s&appid=%s",
             lat, lon, unitsParam, langParam, _apiKey);
    
    String response;
    if (!httpGet(OWM_API_HOST, path, response)) {
        return false;
    }
    
    bool success = parseCurrentWeather(response, weather);
    
    // Update cache on success
    if (success && _cacheDuration > 0) {
        memcpy(&_cachedWeather, weather, sizeof(OWM_CurrentWeather));
        _cachedLat = lat;
        _cachedLon = lon;
        _lastWeatherTime = millis();
        _hasCachedWeather = true;
    }
    
    return success;
}

bool OpenWeatherMap::getCurrentWeatherByCity(const char* cityName, const char* countryCode, 
                                              OWM_CurrentWeather* weather) {
    // First, get coordinates using geocoding
    OWM_GeoLocation location;
    int count = getCoordinatesByName(cityName, countryCode, NULL, &location, 1);
    
    if (count <= 0) {
        setError("City not found");
        return false;
    }
    
    // Then get weather using coordinates
    return getCurrentWeather(location.lat, location.lon, weather);
}

// ============================================================================
// Air Pollution API Implementation
// ============================================================================

bool OpenWeatherMap::getAirPollution(float lat, float lon, OWM_AirPollution* pollution) {
    char path[256];
    snprintf(path, sizeof(path), 
             "/data/2.5/air_pollution?lat=%.4f&lon=%.4f&appid=%s",
             lat, lon, _apiKey);
    
    String response;
    if (!httpGet(OWM_API_HOST, path, response)) {
        return false;
    }
    
    return parseAirPollution(response, pollution);
}

int OpenWeatherMap::getAirPollutionForecast(float lat, float lon, 
                                             OWM_AirPollution* forecast, int maxItems) {
    char path[256];
    snprintf(path, sizeof(path), 
             "/data/2.5/air_pollution/forecast?lat=%.4f&lon=%.4f&appid=%s",
             lat, lon, _apiKey);
    
    String response;
    if (!httpGet(OWM_API_HOST, path, response)) {
        return -1;
    }
    
    return parseAirPollutionList(response, forecast, maxItems);
}

int OpenWeatherMap::getAirPollutionHistory(float lat, float lon, unsigned long startTime, 
                                            unsigned long endTime, OWM_AirPollution* history, 
                                            int maxItems) {
    char path[320];
    snprintf(path, sizeof(path), 
             "/data/2.5/air_pollution/history?lat=%.4f&lon=%.4f&start=%lu&end=%lu&appid=%s",
             lat, lon, startTime, endTime, _apiKey);
    
    String response;
    if (!httpGet(OWM_API_HOST, path, response)) {
        return -1;
    }
    
    return parseAirPollutionList(response, history, maxItems);
}

// ============================================================================
// Forecast API Implementation
// ============================================================================

bool OpenWeatherMap::getForecast(float lat, float lon, OWM_Forecast* forecast, int cnt) {
    char unitsParam[16], langParam[16], cntParam[16];
    buildUnitsParam(unitsParam, sizeof(unitsParam));
    buildLangParam(langParam, sizeof(langParam));
    
    if (cnt > 0) {
        snprintf(cntParam, sizeof(cntParam), "&cnt=%d", cnt);
    } else {
        cntParam[0] = '\0';
    }
    
    char path[256];
    snprintf(path, sizeof(path), 
             "/data/2.5/forecast?lat=%.4f&lon=%.4f%s%s%s&appid=%s",
             lat, lon, unitsParam, langParam, cntParam, _apiKey);
    
    String response;
    if (!httpGet(OWM_API_HOST, path, response)) {
        return false;
    }
    
    return parseForecast(response, forecast);
}

bool OpenWeatherMap::getForecastByCity(const char* cityName, const char* countryCode, 
                                        OWM_Forecast* forecast, int cnt) {
    // First, get coordinates using geocoding
    OWM_GeoLocation location;
    int count = getCoordinatesByName(cityName, countryCode, NULL, &location, 1);
    
    if (count <= 0) {
        setError("City not found");
        return false;
    }
    
    // Then get forecast using coordinates
    return getForecast(location.lat, location.lon, forecast, cnt);
}

// ============================================================================
// Utility Functions
// ============================================================================

const char* OpenWeatherMap::getAQIDescription(int aqi) {
    switch (aqi) {
        case OWM_AQI_GOOD:      return "Good";
        case OWM_AQI_FAIR:      return "Fair";
        case OWM_AQI_MODERATE:  return "Moderate";
        case OWM_AQI_POOR:      return "Poor";
        case OWM_AQI_VERY_POOR: return "Very Poor";
        default:                return "Unknown";
    }
}

char* OpenWeatherMap::getIconURL(const char* iconCode, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%s://openweathermap.org/img/wn/%s@2x.png", 
             _useHttps ? "https" : "http", iconCode);
    return buffer;
}

int OpenWeatherMap::getLastHttpCode() const {
    return _lastHttpCode;
}

const char* OpenWeatherMap::getLastError() const {
    return _lastError;
}

// ============================================================================
// Private Methods - HTTP
// ============================================================================

bool OpenWeatherMap::httpGet(const char* host, const char* path, String& response) {
#if defined(ESP32)
    // ESP32: Use HTTPClient for better performance
    HTTPClient http;
    
    // Build URL
    String url;
    if (_useHttps) {
        url = "https://";
    } else {
        url = "http://";
    }
    url += host;
    url += path;
    
    debugPrint("GET ");
    debugPrintln(url.c_str());
    
    // Configure timeout
    http.setTimeout(_timeout);
    
    if (!http.begin(url)) {
        setError("HTTP begin failed");
        return false;
    }
    
    // Set headers
    http.addHeader("Connection", "close");
    
    // Send request
    _lastHttpCode = http.GET();
    
    debugPrint("HTTP Code: ");
    if (_debug) Serial.println(_lastHttpCode);
    
    if (_lastHttpCode != 200) {
        snprintf(_lastError, sizeof(_lastError), "HTTP Error: %d", _lastHttpCode);
        http.end();
        return false;
    }
    
    // Get response
    response = http.getString();
    http.end();
    
    return true;
    
#elif defined(ARDUINO_UNOWIFIR4)
    // Arduino UNO R4 WiFi: Use manual socket (no HTTPClient available)
    debugPrint("Connecting to ");
    debugPrintln(host);
    
    int port;
    bool connected = false;
    
    if (_useHttps) {
        WiFiSSLClient sslClient;
        port = OWM_API_PORT_HTTPS;
        connected = sslClient.connect(host, port);
        if (!connected) {
            setError("Connection failed");
            return false;
        }
        
        debugPrint("GET ");
        debugPrintln(path);
        
        sslClient.print("GET ");
        sslClient.print(path);
        sslClient.println(" HTTP/1.1");
        sslClient.print("Host: ");
        sslClient.println(host);
        sslClient.println("Connection: close");
        sslClient.println();
        
        unsigned long timeout = millis();
        while (sslClient.available() == 0) {
            if (millis() - timeout > _timeout) {
                setError("Response timeout");
                sslClient.stop();
                return false;
            }
            delay(10);
        }
        
        response = "";
        response.reserve(2048);
        bool headersDone = false;
        String line;
        line.reserve(256);
        char buffer[256];
        
        timeout = millis();
        while (sslClient.connected() || sslClient.available()) {
            if (millis() - timeout > _timeout) {
                setError("Read timeout");
                sslClient.stop();
                return false;
            }
            
            if (sslClient.available()) {
                timeout = millis();
                
                if (!headersDone) {
                    char c = sslClient.read();
                    line += c;
                    if (line.endsWith("\r\n")) {
                        if (line.startsWith("HTTP/")) {
                            int spaceIdx = line.indexOf(' ');
                            if (spaceIdx > 0) {
                                _lastHttpCode = line.substring(spaceIdx + 1, spaceIdx + 4).toInt();
                            }
                        }
                        if (line == "\r\n") {
                            headersDone = true;
                        }
                        line = "";
                    }
                } else {
                    int bytesRead = sslClient.readBytes(buffer, sizeof(buffer) - 1);
                    if (bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        response += buffer;
                    }
                }
            } else {
                delay(1);
            }
        }
        sslClient.stop();
    } else {
        // Use plain HTTP (faster, no SSL handshake)
        WiFiClient httpClient;
        port = OWM_API_PORT_HTTP;
        connected = httpClient.connect(host, port);
        if (!connected) {
            setError("Connection failed");
            return false;
        }
        
        debugPrint("GET ");
        debugPrintln(path);
        
        httpClient.print("GET ");
        httpClient.print(path);
        httpClient.println(" HTTP/1.1");
        httpClient.print("Host: ");
        httpClient.println(host);
        httpClient.println("Connection: close");
        httpClient.println();
        
        unsigned long timeout = millis();
        while (httpClient.available() == 0) {
            if (millis() - timeout > _timeout) {
                setError("Response timeout");
                httpClient.stop();
                return false;
            }
            delay(10);
        }
        
        response = "";
        response.reserve(2048);
        bool headersDone = false;
        String line;
        line.reserve(256);
        char buffer[256];
        
        timeout = millis();
        while (httpClient.connected() || httpClient.available()) {
            if (millis() - timeout > _timeout) {
                setError("Read timeout");
                httpClient.stop();
                return false;
            }
            
            if (httpClient.available()) {
                timeout = millis();
                
                if (!headersDone) {
                    char c = httpClient.read();
                    line += c;
                    if (line.endsWith("\r\n")) {
                        if (line.startsWith("HTTP/")) {
                            int spaceIdx = line.indexOf(' ');
                            if (spaceIdx > 0) {
                                _lastHttpCode = line.substring(spaceIdx + 1, spaceIdx + 4).toInt();
                            }
                        }
                        if (line == "\r\n") {
                            headersDone = true;
                        }
                        line = "";
                    }
                } else {
                    int bytesRead = httpClient.readBytes(buffer, sizeof(buffer) - 1);
                    if (bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        response += buffer;
                    }
                }
            } else {
                delay(1);
            }
        }
        httpClient.stop();
    }
    
    debugPrint("HTTP Code: ");
    if (_debug) Serial.println(_lastHttpCode);
    
    if (_lastHttpCode != 200) {
        snprintf(_lastError, sizeof(_lastError), "HTTP Error: %d", _lastHttpCode);
        return false;
    }
    
    return true;
#endif
}

void OpenWeatherMap::buildUnitsParam(char* buffer, size_t size) {
    switch (_units) {
        case OWM_UNITS_METRIC:
            strncpy(buffer, "&units=metric", size);
            break;
        case OWM_UNITS_IMPERIAL:
            strncpy(buffer, "&units=imperial", size);
            break;
        default:
            buffer[0] = '\0';  // Standard units (default)
            break;
    }
}

void OpenWeatherMap::buildLangParam(char* buffer, size_t size) {
    snprintf(buffer, size, "&lang=%s", _lang);
}

// ============================================================================
// Private Methods - JSON Parsing
// ============================================================================

bool OpenWeatherMap::parseCurrentWeather(const String& json, OWM_CurrentWeather* weather) {
    // Clear the structure
    memset(weather, 0, sizeof(OWM_CurrentWeather));
    
    // Use ArduinoJson to parse
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        setError("JSON parse error");
        debugPrint("JSON Error: ");
        debugPrintln(error.c_str());
        return false;
    }
    
    // Parse coordinates
    weather->lat = doc["coord"]["lat"] | 0.0f;
    weather->lon = doc["coord"]["lon"] | 0.0f;
    
    // Parse weather array (take first item)
    if (doc["weather"].is<JsonArray>() && doc["weather"].size() > 0) {
        JsonObject weatherObj = doc["weather"][0];
        parseWeatherCondition(weatherObj, &weather->weather);
    }
    
    // Parse main data
    JsonObject mainObj = doc["main"];
    parseMainData(mainObj, &weather->main);
    
    // Parse visibility
    weather->visibility = doc["visibility"] | 0;
    
    // Parse wind
    JsonObject windObj = doc["wind"];
    parseWindData(windObj, &weather->wind);
    
    // Parse clouds
    weather->clouds = doc["clouds"]["all"] | 0;
    
    // Parse rain
    weather->rain_1h = doc["rain"]["1h"] | 0.0f;
    
    // Parse snow
    weather->snow_1h = doc["snow"]["1h"] | 0.0f;
    
    // Parse system data
    weather->dt = doc["dt"] | 0UL;
    strncpy(weather->country, doc["sys"]["country"] | "", sizeof(weather->country) - 1);
    weather->sunrise = doc["sys"]["sunrise"] | 0UL;
    weather->sunset = doc["sys"]["sunset"] | 0UL;
    weather->timezone = doc["timezone"] | 0;
    strncpy(weather->name, doc["name"] | "", sizeof(weather->name) - 1);
    
    return true;
}

bool OpenWeatherMap::parseForecast(const String& json, OWM_Forecast* forecast) {
    // Clear the structure
    memset(forecast, 0, sizeof(OWM_Forecast));
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        setError("JSON parse error");
        return false;
    }
    
    // Parse count
    forecast->cnt = doc["cnt"] | 0;
    if (forecast->cnt > OWM_MAX_FORECAST_ITEMS) {
        forecast->cnt = OWM_MAX_FORECAST_ITEMS;
    }
    
    // Parse forecast items
    JsonArray list = doc["list"];
    int index = 0;
    for (JsonObject item : list) {
        if (index >= forecast->cnt) break;
        
        OWM_ForecastItem* fi = &forecast->items[index];
        
        fi->dt = item["dt"] | 0UL;
        
        JsonObject mainObj = item["main"];
        parseMainData(mainObj, &fi->main);
        
        if (item["weather"].is<JsonArray>() && item["weather"].size() > 0) {
            JsonObject weatherObj = item["weather"][0];
            parseWeatherCondition(weatherObj, &fi->weather);
        }
        
        JsonObject windObj = item["wind"];
        parseWindData(windObj, &fi->wind);
        
        fi->clouds = item["clouds"]["all"] | 0;
        fi->visibility = item["visibility"] | 0;
        fi->pop = item["pop"] | 0.0f;
        fi->rain_3h = item["rain"]["3h"] | 0.0f;
        fi->snow_3h = item["snow"]["3h"] | 0.0f;
        strncpy(fi->dt_txt, item["dt_txt"] | "", sizeof(fi->dt_txt) - 1);
        
        index++;
    }
    
    // Parse city info
    JsonObject city = doc["city"];
    strncpy(forecast->city_name, city["name"] | "", sizeof(forecast->city_name) - 1);
    strncpy(forecast->country, city["country"] | "", sizeof(forecast->country) - 1);
    forecast->lat = city["coord"]["lat"] | 0.0f;
    forecast->lon = city["coord"]["lon"] | 0.0f;
    forecast->timezone = city["timezone"] | 0;
    forecast->sunrise = city["sunrise"] | 0UL;
    forecast->sunset = city["sunset"] | 0UL;
    
    return true;
}

bool OpenWeatherMap::parseAirPollution(const String& json, OWM_AirPollution* pollution) {
    memset(pollution, 0, sizeof(OWM_AirPollution));
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        setError("JSON parse error");
        return false;
    }
    
    // Get first item from list
    if (doc["list"].is<JsonArray>() && doc["list"].size() > 0) {
        JsonObject item = doc["list"][0];
        pollution->dt = item["dt"] | 0UL;
        pollution->aqi = item["main"]["aqi"] | 0;
        
        JsonObject comp = item["components"];
        parseAirComponents(comp, &pollution->components);
    }
    
    return true;
}

int OpenWeatherMap::parseAirPollutionList(const String& json, OWM_AirPollution* list, 
                                           int maxItems) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        setError("JSON parse error");
        return -1;
    }
    
    JsonArray jsonList = doc["list"];
    int count = 0;
    
    for (JsonObject item : jsonList) {
        if (count >= maxItems) break;
        
        list[count].dt = item["dt"] | 0UL;
        list[count].aqi = item["main"]["aqi"] | 0;
        
        JsonObject comp = item["components"];
        parseAirComponents(comp, &list[count].components);
        
        count++;
    }
    
    return count;
}

int OpenWeatherMap::parseGeoLocations(const String& json, OWM_GeoLocation* locations, 
                                       int maxResults) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        setError("JSON parse error");
        return -1;
    }
    
    // Response is an array
    if (!doc.is<JsonArray>()) {
        setError("Invalid response format");
        return -1;
    }
    
    JsonArray arr = doc.as<JsonArray>();
    int count = 0;
    
    for (JsonObject item : arr) {
        if (count >= maxResults) break;
        
        strncpy(locations[count].name, item["name"] | "", sizeof(locations[count].name) - 1);
        strncpy(locations[count].country, item["country"] | "", sizeof(locations[count].country) - 1);
        strncpy(locations[count].state, item["state"] | "", sizeof(locations[count].state) - 1);
        locations[count].lat = item["lat"] | 0.0f;
        locations[count].lon = item["lon"] | 0.0f;
        
        count++;
    }
    
    return count;
}

bool OpenWeatherMap::parseGeoZip(const String& json, OWM_GeoLocation* location) {
    memset(location, 0, sizeof(OWM_GeoLocation));
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        setError("JSON parse error");
        return false;
    }
    
    strncpy(location->name, doc["name"] | "", sizeof(location->name) - 1);
    strncpy(location->country, doc["country"] | "", sizeof(location->country) - 1);
    location->lat = doc["lat"] | 0.0f;
    location->lon = doc["lon"] | 0.0f;
    
    return true;
}

void OpenWeatherMap::parseWeatherCondition(JsonObject& obj, OWM_WeatherCondition* condition) {
    condition->id = obj["id"] | 0;
    strncpy(condition->main, obj["main"] | "", sizeof(condition->main) - 1);
    strncpy(condition->description, obj["description"] | "", sizeof(condition->description) - 1);
    strncpy(condition->icon, obj["icon"] | "", sizeof(condition->icon) - 1);
}

void OpenWeatherMap::parseMainData(JsonObject& obj, OWM_MainData* main) {
    main->temp = obj["temp"] | 0.0f;
    main->feels_like = obj["feels_like"] | 0.0f;
    main->temp_min = obj["temp_min"] | 0.0f;
    main->temp_max = obj["temp_max"] | 0.0f;
    main->pressure = obj["pressure"] | 0;
    main->humidity = obj["humidity"] | 0;
    main->sea_level = obj["sea_level"] | 0;
    main->grnd_level = obj["grnd_level"] | 0;
}

void OpenWeatherMap::parseWindData(JsonObject& obj, OWM_WindData* wind) {
    wind->speed = obj["speed"] | 0.0f;
    wind->deg = obj["deg"] | 0;
    wind->gust = obj["gust"] | 0.0f;
}

void OpenWeatherMap::parseAirComponents(JsonObject& obj, OWM_AirComponents* components) {
    components->co = obj["co"] | 0.0f;
    components->no = obj["no"] | 0.0f;
    components->no2 = obj["no2"] | 0.0f;
    components->o3 = obj["o3"] | 0.0f;
    components->so2 = obj["so2"] | 0.0f;
    components->pm2_5 = obj["pm2_5"] | 0.0f;
    components->pm10 = obj["pm10"] | 0.0f;
    components->nh3 = obj["nh3"] | 0.0f;
}

// ============================================================================
// Private Methods - Debug & Error
// ============================================================================

void OpenWeatherMap::debugPrint(const char* message) {
    if (_debug) {
        Serial.print(message);
    }
}

void OpenWeatherMap::debugPrintln(const char* message) {
    if (_debug) {
        Serial.println(message);
    }
}

void OpenWeatherMap::setError(const char* error) {
    strncpy(_lastError, error, sizeof(_lastError) - 1);
    _lastError[sizeof(_lastError) - 1] = '\0';
    debugPrint("Error: ");
    debugPrintln(error);
}
