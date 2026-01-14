# OpenWeatherMap Arduino 库

[![Arduino](https://img.shields.io/badge/Arduino-兼容-blue.svg)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/ESP32-支持-green.svg)](https://www.espressif.com/)
[![License: MIT](https://img.shields.io/badge/许可证-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

一个功能全面的 Arduino OpenWeatherMap API 库，支持多种天气数据接口，使用简单方便。

## 🎯 功能特性

- **实时天气数据** - 获取当前天气状况
- **5天/3小时预报** - 未来5天的天气预测
- **空气污染 API** - 当前、预报及历史空气质量数据
- **地理编码 API** - 城市名称与坐标互相转换

## 🔧 支持的开发板

- **Arduino UNO R4 WiFi**
- **ESP32 系列** (ESP32、ESP32-S2、ESP32-S3、ESP32-C3 等)

## 📦 安装方法

### Arduino 库管理器（推荐）

1. 打开 Arduino IDE
2. 进入 **项目** > **加载库** > **管理库...**
3. 搜索 "OpenWeatherMap"
4. 点击 **安装**

### 手动安装

1. 从 GitHub 下载最新版本
2. 解压到 Arduino 库文件夹：
   - Windows: `文档\Arduino\libraries\`
   - macOS: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`

### PlatformIO

在 `platformio.ini` 中添加：

```ini
lib_deps =
    https://github.com/coloz/OpenWeatherMap-library.git
    bblanchon/ArduinoJson@^7.0.0
```

## 🔑 获取 API 密钥

1. 访问 [OpenWeatherMap](https://openweathermap.org/)
2. 创建免费账户
3. 在账户控制面板中找到 **API Keys**
4. 复制你的 API 密钥

> **注意**: 免费版每天允许 1,000 次 API 调用。

## 📖 快速入门

```cpp
#include <OpenWeatherMap.h>

// 你的配置信息
const char* WIFI_SSID = "你的WiFi名称";
const char* WIFI_PASSWORD = "你的WiFi密码";
const char* API_KEY = "你的OpenWeatherMap_API密钥";

OpenWeatherMap weather;

void setup() {
    Serial.begin(115200);
    
    // 连接 WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    // 初始化库
    weather.begin(API_KEY);
    weather.setUnits(OWM_UNITS_METRIC);  // 使用摄氏度
    weather.setLanguage("zh_cn");         // 使用中文
    
    // 获取当前天气（上海坐标）
    OWM_CurrentWeather data;
    if (weather.getCurrentWeather(31.23f, 121.47f, &data)) {
        Serial.print("温度: ");
        Serial.print(data.main.temp);
        Serial.println("°C");
    }
}

void loop() {
    // 你的代码
}
```

## 📚 API 参考

### 初始化

```cpp
OpenWeatherMap weather;
weather.begin("你的API密钥");
weather.setUnits(OWM_UNITS_METRIC);    // OWM_UNITS_STANDARD(开尔文), OWM_UNITS_METRIC(摄氏度), OWM_UNITS_IMPERIAL(华氏度)
weather.setLanguage("zh_cn");           // 语言代码（如 "en", "zh_cn", "de"）
weather.setTimeout(5000);               // HTTP 请求超时时间，单位毫秒（默认：5000）
weather.setDebug(true);                 // 启用调试输出
```

### 当前天气

```cpp
OWM_CurrentWeather data;

// 通过坐标获取
weather.getCurrentWeather(纬度, 经度, &data);

// 通过城市名获取
weather.getCurrentWeatherByCity("Shanghai", "CN", &data);

// 访问数据
Serial.println(data.main.temp);          // 温度
Serial.println(data.main.humidity);      // 湿度 %
Serial.println(data.weather.description); // "晴天"等
Serial.println(data.wind.speed);         // 风速
```

### 5天天气预报

```cpp
OWM_Forecast forecast;

// 获取所有预报（最多40条）
weather.getForecast(纬度, 经度, &forecast);

// 获取有限的预报数量
weather.getForecast(纬度, 经度, &forecast, 8);  // 24小时内

// 通过城市名获取
weather.getForecastByCity("Beijing", "CN", &forecast);

// 访问数据
for (int i = 0; i < forecast.cnt; i++) {
    Serial.print(forecast.items[i].dt_txt);
    Serial.print(": ");
    Serial.print(forecast.items[i].main.temp);
    Serial.println("°C");
}
```

### 空气污染

```cpp
OWM_AirPollution pollution;

// 当前空气污染
weather.getAirPollution(纬度, 经度, &pollution);

// 空气污染预报
OWM_AirPollution forecast[24];
int count = weather.getAirPollutionForecast(纬度, 经度, forecast, 24);

// 历史数据
int count = weather.getAirPollutionHistory(纬度, 经度, 开始时间, 结束时间, history, 100);

// 访问数据
Serial.println(pollution.aqi);               // 空气质量指数 (1-5)
Serial.println(pollution.components.pm2_5);  // PM2.5
Serial.println(pollution.components.pm10);   // PM10
Serial.println(OpenWeatherMap::getAQIDescription(pollution.aqi));
```

### 地理编码

```cpp
OWM_GeoLocation locations[5];

// 城市名转坐标
int count = weather.getCoordinatesByName("Shanghai", "CN", NULL, locations, 5);

// 邮政编码转坐标
OWM_GeoLocation loc;
weather.getCoordinatesByZip("200000", "CN", &loc);

// 坐标转地名（反向地理编码）
count = weather.getLocationByCoordinates(31.2304f, 121.4737f, locations, 5);

// 访问数据
Serial.println(locations[0].name);
Serial.println(locations[0].lat);
Serial.println(locations[0].lon);
```

## 📊 数据结构

### OWM_CurrentWeather（当前天气）

| 字段 | 类型 | 描述 |
|------|------|------|
| `name` | char[] | 城市名称 |
| `country` | char[] | 国家代码 |
| `lat`, `lon` | float | 经纬度坐标 |
| `main.temp` | float | 温度 |
| `main.feels_like` | float | 体感温度 |
| `main.humidity` | int | 湿度 % |
| `main.pressure` | int | 气压 (hPa) |
| `weather.main` | char[] | 天气类型 |
| `weather.description` | char[] | 天气描述 |
| `weather.icon` | char[] | 图标代码 |
| `wind.speed` | float | 风速 |
| `wind.deg` | int | 风向 |
| `visibility` | int | 能见度 (米) |
| `clouds` | int | 云量 % |
| `sunrise`, `sunset` | unsigned long | 日出日落时间戳 |

### OWM_AirPollution（空气污染）

| 字段 | 类型 | 描述 |
|------|------|------|
| `aqi` | int | 空气质量指数 (1-5) |
| `components.co` | float | 一氧化碳 (μg/m³) |
| `components.no` | float | 一氧化氮 (μg/m³) |
| `components.no2` | float | 二氧化氮 (μg/m³) |
| `components.o3` | float | 臭氧 (μg/m³) |
| `components.so2` | float | 二氧化硫 (μg/m³) |
| `components.pm2_5` | float | PM2.5 (μg/m³) |
| `components.pm10` | float | PM10 (μg/m³) |
| `components.nh3` | float | 氨 (μg/m³) |

### 空气质量指数等级

| AQI | 等级 | 描述 |
|-----|------|------|
| 1 | 优 | 空气质量令人满意 |
| 2 | 良 | 空气质量可接受 |
| 3 | 中等 | 敏感人群可能受影响 |
| 4 | 差 | 所有人都可能受到健康影响 |
| 5 | 非常差 | 健康警告 - 避免户外活动 |

## 🌐 支持的语言

使用 `weather.setLanguage("代码")` 设置语言：

| 代码 | 语言 | 代码 | 语言 |
|------|------|------|------|
| `zh_cn` | 简体中文 | `en` | 英语 |
| `zh_tw` | 繁体中文 | `de` | 德语 |
| `ja` | 日语 | `fr` | 法语 |
| `kr` | 韩语 | `es` | 西班牙语 |
| `ru` | 俄语 | `pt` | 葡萄牙语 |
| `ar` | 阿拉伯语 | `it` | 意大利语 |

[完整语言列表](https://openweathermap.org/current#multi)

## 📝 示例程序

库中包含多个示例：

- **CurrentWeather** - 基本当前天气获取
- **Forecast5Day** - 5天预报（3小时间隔）
- **AirPollution** - 空气质量监测
- **Geocoding** - 地理位置查询和反向编码
- **CompleteExample** - 完整功能的气象站示例

## ⚠️ 故障排除

### 连接问题

- 确保 WiFi 账号密码正确
- 检查 API 密钥是否有效且已激活
- 验证网络连接是否正常

### 内存问题

- ESP32 有足够的内存支持所有功能
- Arduino UNO R4 WiFi：建议使用 `cnt` 参数限制预报数量
- 生产环境中使用 `weather.setDebug(false)` 节省内存

### API 调用限制

- 免费版：每天 1,000 次调用，每分钟 60 次
- 在 API 调用之间添加适当延迟
- 尽可能本地缓存数据

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

## 🙏 致谢

- [OpenWeatherMap](https://openweathermap.org/) 提供天气 API
- [ArduinoJson](https://arduinojson.org/) 作者 Benoît Blanchon

## 📮 支持与反馈

- 发现 Bug 请创建 [Issue](https://github.com/coloz/OpenWeatherMap-library/issues)
- 提交前请先检查现有的 Issues
- 欢迎提交 Pull Request！

---

## 🚀 更多示例

### 获取北京空气质量

```cpp
void getBeijingAirQuality() {
    OWM_AirPollution aqi;
    
    // 北京坐标
    if (weather.getAirPollution(39.9042f, 116.4074f, &aqi)) {
        Serial.print("北京空气质量指数: ");
        Serial.println(aqi.aqi);
        
        Serial.print("PM2.5: ");
        Serial.print(aqi.components.pm2_5);
        Serial.println(" μg/m³");
        
        Serial.print("PM10: ");
        Serial.print(aqi.components.pm10);
        Serial.println(" μg/m³");
    }
}
```

### 获取上海未来天气

```cpp
void getShanghaiWeather() {
    OWM_Forecast forecast;
    
    // 上海坐标，获取8个时间点（24小时）
    if (weather.getForecast(31.2304f, 121.4737f, &forecast, 8)) {
        Serial.println("上海未来24小时天气：");
        
        for (int i = 0; i < forecast.cnt; i++) {
            Serial.print(forecast.items[i].dt_txt);
            Serial.print(" - ");
            Serial.print(forecast.items[i].main.temp, 1);
            Serial.print("°C, ");
            Serial.println(forecast.items[i].weather.description);
        }
    }
}
```

### 城市搜索示例

```cpp
void searchCity(const char* cityName) {
    OWM_GeoLocation results[5];
    
    int count = weather.getCoordinatesByName(cityName, NULL, NULL, results, 5);
    
    if (count > 0) {
        Serial.print("找到 ");
        Serial.print(count);
        Serial.println(" 个结果：");
        
        for (int i = 0; i < count; i++) {
            Serial.print(i + 1);
            Serial.print(". ");
            Serial.print(results[i].name);
            Serial.print(", ");
            Serial.print(results[i].country);
            Serial.print(" (");
            Serial.print(results[i].lat, 4);
            Serial.print(", ");
            Serial.print(results[i].lon, 4);
            Serial.println(")");
        }
    }
}
```
