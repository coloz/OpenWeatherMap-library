# OpenWeatherMap Arduino Library

[![Arduino](https://img.shields.io/badge/Arduino-Compatible-blue.svg)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/ESP32-Supported-green.svg)](https://www.espressif.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A comprehensive Arduino library for accessing OpenWeatherMap APIs. Supports multiple weather data endpoints with an easy-to-use interface.

## 🎯 Features

- **Current Weather Data** - Get real-time weather conditions
- **5-Day / 3-Hour Forecast** - Weather predictions for the next 5 days
- **Air Pollution API** - Current, forecast, and historical air quality data
- **Geocoding API** - Convert city names to coordinates and vice versa

## 🔧 Supported Boards

- **Arduino UNO R4 WiFi**
- **ESP32 series** (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, etc.)

## 📦 Installation

### Arduino Library Manager (Recommended)

1. Open Arduino IDE
2. Go to **Sketch** > **Include Library** > **Manage Libraries...**
3. Search for "OpenWeatherMap"
4. Click **Install**

### Manual Installation

1. Download the latest release from GitHub
2. Extract to your Arduino libraries folder:
   - Windows: `Documents\Arduino\libraries\`
   - macOS: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/coloz/OpenWeatherMap-library.git
    bblanchon/ArduinoJson@^7.0.0
```

## 🔑 Getting an API Key

1. Go to [OpenWeatherMap](https://openweathermap.org/)
2. Create a free account
3. Navigate to **API Keys** in your account dashboard
4. Copy your API key

> **Note**: Free tier allows 1,000 API calls per day.

## 📖 Quick Start

```cpp
#include <OpenWeatherMap.h>

// Your credentials
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
const char* API_KEY = "your_openweathermap_api_key";

OpenWeatherMap weather;

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    // Initialize library
    weather.begin(API_KEY);
    weather.setUnits(OWM_UNITS_METRIC);  // Celsius
    weather.setLanguage("en");
    
    // Get current weather
    OWM_CurrentWeather data;
    if (weather.getCurrentWeather(31.23f, 121.47f, &data)) {
        Serial.print("Temperature: ");
        Serial.print(data.main.temp);
        Serial.println("°C");
    }
}

void loop() {
    // Your code here
}
```

## 📚 API Reference

### Initialization

```cpp
OpenWeatherMap weather;
weather.begin("YOUR_API_KEY");
weather.setUnits(OWM_UNITS_METRIC);    // OWM_UNITS_STANDARD, OWM_UNITS_METRIC, OWM_UNITS_IMPERIAL
weather.setLanguage("en");              // Language code (e.g., "en", "zh_cn", "de")
weather.setDebug(true);                 // Enable debug output
```

### Current Weather

```cpp
OWM_CurrentWeather data;

// By coordinates
weather.getCurrentWeather(latitude, longitude, &data);

// By city name
weather.getCurrentWeatherByCity("London", "GB", &data);

// Access data
Serial.println(data.main.temp);          // Temperature
Serial.println(data.main.humidity);      // Humidity %
Serial.println(data.weather.description); // "clear sky", etc.
Serial.println(data.wind.speed);         // Wind speed
```

### 5-Day Forecast

```cpp
OWM_Forecast forecast;

// Get all forecasts (up to 40 items)
weather.getForecast(latitude, longitude, &forecast);

// Get limited forecasts
weather.getForecast(latitude, longitude, &forecast, 8);  // 24 hours

// By city name
weather.getForecastByCity("Tokyo", "JP", &forecast);

// Access data
for (int i = 0; i < forecast.cnt; i++) {
    Serial.print(forecast.items[i].dt_txt);
    Serial.print(": ");
    Serial.print(forecast.items[i].main.temp);
    Serial.println("°C");
}
```

### Air Pollution

```cpp
OWM_AirPollution pollution;

// Current air pollution
weather.getAirPollution(latitude, longitude, &pollution);

// Air pollution forecast
OWM_AirPollution forecast[24];
int count = weather.getAirPollutionForecast(lat, lon, forecast, 24);

// Historical data
int count = weather.getAirPollutionHistory(lat, lon, startTime, endTime, history, 100);

// Access data
Serial.println(pollution.aqi);           // Air Quality Index (1-5)
Serial.println(pollution.components.pm2_5);  // PM2.5
Serial.println(pollution.components.pm10);   // PM10
Serial.println(OpenWeatherMap::getAQIDescription(pollution.aqi));
```

### Geocoding

```cpp
OWM_GeoLocation locations[5];

// City name to coordinates
int count = weather.getCoordinatesByName("Paris", "FR", NULL, locations, 5);

// ZIP code to coordinates
OWM_GeoLocation loc;
weather.getCoordinatesByZip("10001", "US", &loc);

// Coordinates to location name (reverse geocoding)
count = weather.getLocationByCoordinates(48.8566f, 2.3522f, locations, 5);

// Access data
Serial.println(locations[0].name);
Serial.println(locations[0].lat);
Serial.println(locations[0].lon);
```

## 📊 Data Structures

### OWM_CurrentWeather

| Field | Type | Description |
|-------|------|-------------|
| `name` | char[] | City name |
| `country` | char[] | Country code |
| `lat`, `lon` | float | Coordinates |
| `main.temp` | float | Temperature |
| `main.feels_like` | float | Feels like temperature |
| `main.humidity` | int | Humidity % |
| `main.pressure` | int | Pressure (hPa) |
| `weather.main` | char[] | Weather group |
| `weather.description` | char[] | Description |
| `weather.icon` | char[] | Icon code |
| `wind.speed` | float | Wind speed |
| `wind.deg` | int | Wind direction |
| `visibility` | int | Visibility (m) |
| `clouds` | int | Cloudiness % |
| `sunrise`, `sunset` | unsigned long | Unix timestamps |

### OWM_AirPollution

| Field | Type | Description |
|-------|------|-------------|
| `aqi` | int | Air Quality Index (1-5) |
| `components.co` | float | Carbon monoxide (μg/m³) |
| `components.no` | float | Nitrogen monoxide (μg/m³) |
| `components.no2` | float | Nitrogen dioxide (μg/m³) |
| `components.o3` | float | Ozone (μg/m³) |
| `components.so2` | float | Sulphur dioxide (μg/m³) |
| `components.pm2_5` | float | PM2.5 (μg/m³) |
| `components.pm10` | float | PM10 (μg/m³) |
| `components.nh3` | float | Ammonia (μg/m³) |

### Air Quality Index Levels

| AQI | Level | Description |
|-----|-------|-------------|
| 1 | Good | Air quality is satisfactory |
| 2 | Fair | Air quality is acceptable |
| 3 | Moderate | May affect sensitive individuals |
| 4 | Poor | Health effects possible for everyone |
| 5 | Very Poor | Health warnings - avoid outdoor activities |

## 🌐 Supported Languages

Use `weather.setLanguage("code")` with these codes:

| Code | Language | Code | Language |
|------|----------|------|----------|
| `en` | English | `zh_cn` | Chinese Simplified |
| `de` | German | `zh_tw` | Chinese Traditional |
| `fr` | French | `ja` | Japanese |
| `es` | Spanish | `kr` | Korean |
| `pt` | Portuguese | `ru` | Russian |
| `it` | Italian | `ar` | Arabic |

[Full language list](https://openweathermap.org/current#multi)

## 📝 Examples

The library includes several examples:

- **CurrentWeather** - Basic current weather retrieval
- **Forecast5Day** - 5-day forecast with 3-hour intervals
- **AirPollution** - Air quality monitoring
- **Geocoding** - Location lookup and reverse geocoding
- **CompleteExample** - Full-featured weather station

## ⚠️ Troubleshooting

### Connection Issues

- Ensure WiFi credentials are correct
- Check that your API key is valid and active
- Verify internet connectivity

### Memory Issues

- ESP32 has plenty of RAM for all features
- Arduino UNO R4 WiFi: Consider limiting forecast items with `cnt` parameter
- Use `weather.setDebug(false)` in production to save memory

### API Rate Limits

- Free tier: 1,000 calls/day, 60 calls/minute
- Implement appropriate delays between API calls
- Cache data locally when possible

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [OpenWeatherMap](https://openweathermap.org/) for providing the weather API
- [ArduinoJson](https://arduinojson.org/) by Benoît Blanchon

## 📮 Support

- Create an [issue](https://github.com/yourusername/OpenWeatherMap-Arduino/issues) for bugs
- Check existing issues before creating new ones
- Pull requests are welcome!
