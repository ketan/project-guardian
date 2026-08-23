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

Current pinout

![ESP32 Pinout](hardware/pinout.png)
