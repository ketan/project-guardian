# Current hardware design

```mermaid
flowchart TD
    SOLAR["12V Solar Panel"]

    ESP["ESP32-S3 SIM7670G Board<br/>Solar charger + LiPo mgmt"]
    BAT["LiPo / 18650 Battery"]

    BOOST["12V Boost Converter<br/>TPS61040 / AP3012"]

    ID1["Ideal Diode #1<br/>(Solar path)<br/>LM74610 / module/Polulu 5398"]
    ID2["Ideal Diode #2<br/>(Battery path)<br/>LM74610 / module/Polulu 5398"]

    RAIL["Combined 12V Rail"]

    SEN["SEN0658 Sensor<br/>10–30V, RS485"]

    RS485["MAX3485 TTL ↔ RS485 Module<br/>(3.3V logic)"]

    %% Charging path
    SOLAR --> ESP
    ESP --> BAT

    %% Solar direct power path
    SOLAR --> ID1

    %% Battery path
    BAT --> BOOST
    BOOST --> ID2

    %% OR-ing
    ID1 --> RAIL
    ID2 --> RAIL

    %% Load
    RAIL --> SEN

    %% Boost converter enable
    ESP -->|"GPIO_ENABLE_PIN<br/>(to reduce power consumption at night)"| BOOST

    %% RS485 comms
    ESP -->|UART TX/RX| RS485
    RS485 -->|A/B| SEN

    %% RS485 power
    ESP -->|3.3V| RS485
```


Possible future improvements:

* Enable/Disable MAX3485 (using a logic level P-Channel MOSFET) via GPIO to further reduce power consumption when not communicating
* Addition of an RTC module for accurate timekeeping (e.g., DS3231) to timestamp sensor data

## Current pinout

We use waveshare ESP32-S3 SIM7670G board (https://www.waveshare.com/esp32-s3-sim7670g-4g.htm). Part number (ESP32-S3-SIM7670G-4G-EN) and the pinout is as follows:

![ESP32 Pinout](hardware/pinout.png)

## Hardware notes/observations

- board defaults to 5/6V solar charging input. Need to change jumpers to support higher input voltages
- Battery charge current is set to 1.2A using CN3791.

## Low-power operation

- Poll wind at 1 Hz using ESP32 light sleep between reads; do not deep-sleep each second.
- Store one five-minute aggregate on SD and upload it to Windy every five minutes.
- Disable Wi-Fi, Bluetooth, mDNS, captive portal, and Telnet in deployed LTE-only operation.
- Put the SIM7670G into LTE PSM between uploads when the carrier supports it; measure this against power-cycling the modem.
- Measure current for sampling, modem-idle/PSM, and upload states before finalizing the duty cycle.

## Wind aggregation

Poll wind speed and direction at 1 Hz. Report sustained wind as a rolling two-minute vector average: convert each speed/direction pair into north/east components, average them, then convert back to speed and direction. This correctly handles directions around north (for example, 359° and 1° average to 0°, not 180°).

Report gust as the highest short-running speed average since the previous five-minute upload. Keeping the rolling average avoids reporting a single noisy reading as a gust while preserving short, meaningful peaks.
