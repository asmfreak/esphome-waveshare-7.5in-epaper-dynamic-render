#include "epaper.hpp"
#include "elements.hpp"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include <algorithm>



namespace esphome {
namespace waveshare_epaper {

static const char *TAG = "waveshare_epaper";


void WaveshareEPaper::setup_pins_() {
    // this->init_internal_(this->get_buffer_length_());
    this->dc_pin_->setup();  // OUTPUT
    this->dc_pin_->digital_write(false);
    if (this->reset_pin_ != nullptr) {
        this->reset_pin_->setup();  // OUTPUT
        this->reset_pin_->digital_write(true);
    }
    if (this->busy_pin_ != nullptr) {
        this->busy_pin_->setup();  // INPUT
    }
    this->spi_setup();
    
    this->reset_();
}
float WaveshareEPaper::get_setup_priority() const { return setup_priority::PROCESSOR; }
void WaveshareEPaper::command(uint8_t value) {
    this->start_command_();
    this->write_byte(value);
    this->end_command_();
}
void WaveshareEPaper::data(uint8_t value) {
    this->start_data_();
    this->write_byte(value);
    this->end_data_();
}
bool WaveshareEPaper::wait_until_idle_() {
    if (this->busy_pin_ == nullptr) {
        return true;
    }
    
    const uint32_t start = millis();
    while (this->busy_pin_->digital_read()) {
        if (millis() - start > 1000) {
            ESP_LOGE(TAG, "Timeout while displaying image!");
            return false;
        }
        delay(10);
    }
    return true;
}
void WaveshareEPaper::update() {
    if (ready_to_update){
        this->do_update_();
        this->display();
    }
}

void WaveshareEPaper::start_command_() {
    this->dc_pin_->digital_write(false);
    this->enable();
}
void WaveshareEPaper::end_command_() { this->disable(); }
void WaveshareEPaper::start_data_() {
    this->dc_pin_->digital_write(true);
    this->enable();
}
void WaveshareEPaper::end_data_() { this->disable(); }
void WaveshareEPaper::on_safe_shutdown() { this->deep_sleep(); }


// ========================================================
//                          Type B
// ========================================================
// Datasheet:
//  - https://www.waveshare.com/w/upload/7/7f/4.2inch-e-paper-b-specification.pdf
//  - https://github.com/soonuse/epd-library-arduino/blob/master/4.2inch_e-paper/epd4in2/



void WaveshareEPaper7P5InC::initialize() {
    // COMMAND POWER SETTING
    ESP_LOGW(TAG, "COMMAND POWER SETTING");
    this->command(0x01);
    this->data(0x37);
    this->data(0x00);
    
    // COMMAND PANEL SETTING
    ESP_LOGW(TAG, "COMMAND PANEL SETTING");
    this->command(0x00);
    this->data(0xCF);
    // this->data(0x0B);
    this->data(0x08);
    
    // COMMAND BOOSTER SOFT START
    ESP_LOGW(TAG, "COMMAND BOOSTER SOFT START");
    this->command(0x06);
    this->data(0xC7);
    this->data(0xCC);
    this->data(0x28);
    
    // COMMAND POWER ON
    ESP_LOGW(TAG, "COMMAND POWER ON");
    this->command(0x04);
    this->wait_until_idle_();
    delay(10);
    
    // COMMAND PLL CONTROL
    ESP_LOGW(TAG, "COMMAND PLL CONTROL");
    this->command(0x30);
    this->data(0x3C);
    
    // COMMAND TEMPERATURE SENSOR CALIBRATION
    ESP_LOGW(TAG, "COMMAND TEMPERATURE SENSOR CALIBRATION");
    this->command(0x41);
    this->data(0x00);
    
    // COMMAND VCOM AND DATA INTERVAL SETTING
    ESP_LOGW(TAG, "COMMAND VCOM AND DATA INTERVAL SETTING");
    this->command(0x50);
    this->data(0x77);
    
    // COMMAND TCON SETTING
    ESP_LOGW(TAG, "COMMAND TCON SETTING");
    this->command(0x60);
    this->data(0x22);
    
    // COMMAND RESOLUTION SETTING
    ESP_LOGW(TAG, "COMMAND RESOLUTION SETTING");
    this->command(0x61);
    this->data(0x02);
    this->data(0x80);
    this->data(0x01);
    this->data(0x80);
    
    // COMMAND VCM DC SETTING REGISTER
    ESP_LOGW(TAG, "COMMAND VCM DC SETTING REGISTER");
    this->command(0x82);
    this->data(0x1E);
    
    this->command(0xE5);
    this->data(0x03);
}

void HOT WaveshareEPaper7P5InC::display() {
    // COMMAND DATA START TRANSMISSION 1
    this->command(0x10);
    
    this->start_data_();
    
    uint8_t pix2 = 0;
    bool pi = true;
    elements.render([this, &pi, &pix2](int x, int y, const elements::Color3& pallettePix){
        if(x==0){
            ESP_LOGW(TAG, "Render line %d of %d", y, static_height_());
        }
        auto binPix = elements::col2bin(pallettePix);
        if (pi){
            pix2 = pix2 | binPix.color;
        } else {
            pix2 = pix2 <<4;
            pix2 = pix2 | binPix.color;
            this->write_byte(pix2);
        }
        pi = !pi;
        App.feed_wdt();
    });
    App.feed_wdt();
    
    
    this->end_data_();
    
    // COMMAND DISPLAY REFRESH
    ESP_LOGW(TAG, "COMMAND DISPLAY REFRESH");
    this->command(0x12);
    // this->wait_until_idle_();
}

void WaveshareEPaper7P5InC::fill(Color color) {
    clear();
    elements.fill(elements::Color3{color});
}

void WaveshareEPaper7P5InC::clear(){
    elements.clear();
}

uint32_t WaveshareEPaper7P5InC::get_buffer_length_(){ return 0; }
int WaveshareEPaper7P5InC::get_width_internal() { return static_width_(); }
int WaveshareEPaper7P5InC::get_height_internal() { return static_height_(); }


void WaveshareEPaper7P5InC::dump_config() {
    LOG_DISPLAY("", "Waveshare E-Paper", this);
    ESP_LOGCONFIG(TAG, "  Model: 7.5in_c");
    LOG_PIN("  Reset Pin: ", this->reset_pin_);
    LOG_PIN("  DC Pin: ", this->dc_pin_);
    LOG_PIN("  Busy Pin: ", this->busy_pin_);
    LOG_UPDATE_INTERVAL(this);
}



}  // namespace waveshare_epaper
}  // namespace esphome
