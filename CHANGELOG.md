# 更新日志 / Changelog

所有重要的项目变更都将记录在此文件中。

## [1.0.0] - 2026-01-08

### 新增功能
- **Geocoding API 支持**
  - 城市名称转坐标 (Direct Geocoding)
  - 邮政编码转坐标
  - 坐标转地名 (Reverse Geocoding)
  
- **当前天气 API**
  - 通过坐标获取天气
  - 通过城市名获取天气
  - 支持温度、湿度、气压、风速等数据
  
- **空气污染 API**
  - 当前空气质量数据
  - 空气质量预报（最多4天）
  - 历史空气质量数据
  - AQI 指数及各污染物浓度
  
- **5天天气预报 API**
  - 3小时间隔的天气预报
  - 最多40个时间点数据
  - 降水概率

- **多平台支持**
  - Arduino UNO R4 WiFi
  - ESP32 全系列

- **便捷功能**
  - 支持公制/英制/标准单位切换
  - 多语言支持
  - 调试模式
  - 错误信息获取

### 示例程序
- CurrentWeather - 当前天气示例
- Forecast5Day - 5天预报示例
- AirPollution - 空气质量示例
- Geocoding - 地理编码示例
- CompleteExample - 完整功能示例
