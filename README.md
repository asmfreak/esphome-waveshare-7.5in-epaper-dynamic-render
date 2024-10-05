# esphome-waveshare-7.5in-epaper-dynamic-render

This repo is an [external component for `esphome`](https://esphome.io/components/external_components.html) for Waveshare 7.5 inch epaper display.
It is targets specifically ESP8622 module, because, due to ESP8266 RAM size, vanilla esphome component cant allocate a big enough frame buffer.
This repo uses an alternative rendering engine - it stores draw commands, which are evaluated when data is sent to display.

> [!WARNING]  
> This rendering algorithm right now is EXTREMELY slow.
> It can take a LOT of time (minutes) to render a screen with lots of grafical elements. 
> It, though, pales in comparison with display update time (2 minutes).


## Usage

```
cd path/to/your/configs
mkdir components
cd components
git clone https://github.com/asmfreak/esphome-waveshare-7.5in-epaper-dynamic-render.git

```


In your yaml add the following lines:

```
esphome:
  on_boot: 
    then:
      - wait_until:
          wifi.connected:
      - lambda: |-
          ESP_LOGW("main", "on boot");
          id(colDisp).set_ready_for_updates();
          id(colDisp).update();


external_components:
  - source: 
      path: ./components
      type: local

font:
  # gfonts://family[@weight]
  - file: "gfonts://Roboto"
    id: roboto_20
    size: 30

image:
  - file: mdi:alert-outline
    id: alert
    resize: 80x80


spi:
  - id: spi0
    mosi_pin: GPIO13
    clk_pin: GPIO14


display:
  - platform: epaper
    id: colDisp
    spi_id: spi0
    dc_pin: GPIO4
    cs_pin: GPIO15
    reset_pin: GPIO2
    busy_pin: GPIO5
    lambda: |
        using namespace esphome::display;
        it.fill(COLOR_OFF);
        it.print(
            30, it.static_height_()/2+30, roboto_20, COLOR_OFF,
            TextAlign::TOP_LEFT, "hello world", COLOR_ON);
```
