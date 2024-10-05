#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display.h"
#include "esphome/components/spi/spi.h"
#include "elements.hpp"

namespace esphome {
namespace waveshare_epaper {

class WaveshareEPaper
    : public esphome::display::Display
    , public esphome::spi::SPIDevice<
        spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
        spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_2MHZ
    > {
public:
    void set_dc_pin(GPIOPin *dc_pin) { dc_pin_ = dc_pin; }
    float get_setup_priority() const override;
    void set_reset_pin(GPIOPin *reset) { this->reset_pin_ = reset; }
    void set_busy_pin(GPIOPin *busy) { this->busy_pin_ = busy; }

    void command(uint8_t value);
    void data(uint8_t value);

    virtual void display() = 0;
    virtual void initialize() = 0;
    virtual void deep_sleep() = 0;

    void update() override;

    void setup() override {
        ready_to_update = false;
        this->setup_pins_();
        this->initialize();
    }

    void on_safe_shutdown() override;
    
    void set_ready_for_updates(bool r=true){
        ready_to_update = r;
    }

protected:
    // void draw_absolute_pixel_internal(int x, int y, int color) override;

    bool wait_until_idle_();

    void setup_pins_();

    void reset_() {
        if (this->reset_pin_ != nullptr) {
        this->reset_pin_->digital_write(false);
        delay(200);
        this->reset_pin_->digital_write(true);
        delay(200);
        }
    }

    virtual uint32_t get_buffer_length_();

    void start_command_();
    void end_command_();
    void start_data_();
    void end_data_();

    GPIOPin *reset_pin_{nullptr};
    GPIOPin *dc_pin_;
    GPIOPin *busy_pin_{nullptr};
    
    bool ready_to_update;
};


namespace detail{
struct WaveshareEPaper7P5InCProps{
    constexpr static int static_width_(){  return 640;}
    constexpr static int static_height_(){ return 384; }
};
}

class WaveshareEPaper7P5InC;

using epaper_writer_t =  std::function<void(WaveshareEPaper7P5InC &)>;

class WaveshareEPaper7P5InC final :
    public WaveshareEPaper,
    public detail::WaveshareEPaper7P5InCProps
{
public:
    void initialize() override;

    void display() override;

    void dump_config() override;

    void deep_sleep() override {
        // COMMAND POWER OFF
        this->command(0x02);
        this->wait_until_idle_();
        // COMMAND DEEP SLEEP
        this->command(0x07);
        this->data(0xA5);  // check byte
    }

    display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }

    void set_writer(epaper_writer_t&& w) {
        display::Display::set_writer([this, w](display::Display& d){
            w(*this);
        });
    }

    void fill(Color color) override;
    void clear();
    
    inline void draw_pixel_at(int x, int y){
        this->elements.draw_pixel_at(x, y);
    }
    void draw_pixel_at(int x, int y, Color color) override {
        this->elements.draw_pixel_at(x, y, color);
    }

    void draw_pixels_at(int x_start, int y_start, int w, int h, const uint8_t *ptr, display::ColorOrder order, display::ColorBitness bitness, bool big_endian, int x_offset, int y_offset, int x_pad) override {
        this->elements.draw_pixels_at(x_start, y_start, w, h, ptr, order, bitness, big_endian, x_offset, y_offset, x_pad);
    }

    void line(int x1, int y1, int x2, int y2, Color color = display::COLOR_ON){
        this->elements.line(x1, y1, x2, y2, color);
    }

    void line_at_angle(int x, int y, int angle, int length, Color color = display::COLOR_ON){
        this->elements.line_at_angle(x, y, angle, length, color);
    }

    void line_at_angle(int x, int y, int angle, int start_radius, int stop_radius, Color color = display::COLOR_ON){
        this->elements.line_at_angle(x, y, angle, start_radius, stop_radius, color);
    }

    void horizontal_line(int x, int y, int width, Color color = display::COLOR_ON){
        this->elements.horizontal_line(x, y, width, color);
    }

    void vertical_line(int x, int y, int height, Color color = display::COLOR_ON){
        this->elements.vertical_line(x, y, height, color);
    }

    void rectangle(int x1, int y1, int width, int height, Color color = display::COLOR_ON){
        this->elements.rectangle(x1, y1, width, height, color);
    }

    void filled_rectangle(int x1, int y1, int width, int height, Color color = display::COLOR_ON){
        this->elements.filled_rectangle(x1, y1, width, height, color);
    }

    void circle(int center_x, int center_xy, int radius, Color color = display::COLOR_ON){
        this->elements.circle(center_x, center_xy, radius, color);
    }

    void filled_circle(int center_x, int center_y, int radius, Color color = display::COLOR_ON){
        this->elements.filled_circle(center_x, center_y, radius, color);
    }

    void triangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color = display::COLOR_ON){
        this->elements.triangle(x1, y1, x2, y2, x3, y3, color);
    }

    void filled_triangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color = display::COLOR_ON){
        this->elements.filled_triangle(x1, y1, x2, y2, x3, y3, color);
    }

    void get_regular_polygon_vertex(int vertex_id, int *vertex_x, int *vertex_y, int center_x, int center_y, int radius,int edges, display::RegularPolygonVariation variation = display::VARIATION_POINTY_TOP,float rotation_degrees = display::ROTATION_0_DEGREES){
        this->elements.get_regular_polygon_vertex(vertex_id, vertex_x, vertex_y, center_x, center_y, radius, edges, variation, rotation_degrees);
    }

    void regular_polygon(int x, int y, int radius, int edges, display::RegularPolygonVariation variation = display::VARIATION_POINTY_TOP,float rotation_degrees = display::ROTATION_0_DEGREES, Color color = display::COLOR_ON,display::RegularPolygonDrawing drawing = display::DRAWING_OUTLINE){
        this->elements.regular_polygon(x, y, radius, edges, variation, rotation_degrees, color, drawing);
    }

    void regular_polygon(int x, int y, int radius, int edges, display::RegularPolygonVariation variation, Color color,display::RegularPolygonDrawing drawing = display::DRAWING_OUTLINE){
        this->elements.regular_polygon(x, y, radius, edges, variation, color, drawing);
    }

    void regular_polygon(int x, int y, int radius, int edges, Color color,display::RegularPolygonDrawing drawing = display::DRAWING_OUTLINE){
        this->elements.regular_polygon(x, y, radius, edges, color,drawing);
    }

    void filled_regular_polygon(int x, int y, int radius, int edges, display::RegularPolygonVariation variation = display::VARIATION_POINTY_TOP, float rotation_degrees = display::ROTATION_0_DEGREES, Color color = display::COLOR_ON){
        this->elements.filled_regular_polygon(x, y, radius, edges, variation, rotation_degrees, color);
    }

    void filled_regular_polygon(int x, int y, int radius, int edges, display::RegularPolygonVariation variation, Color color){
        this->elements.filled_regular_polygon(x, y, radius, edges, variation, color);
    }

    void filled_regular_polygon(int x, int y, int radius, int edges, Color color){
        this->elements.filled_regular_polygon(x, y, radius, edges, color);
    }

    void print(int x, int y, font::Font *font, Color color, display::TextAlign align, const char *text, Color background = display::COLOR_OFF){
        this->elements.print(x, y, font, color, align, text, background);
    }

    void print(int x, int y, font::Font *font, Color color, const char *text, Color background = display::COLOR_OFF){
        this->elements.print(x, y, font, color, text, background);
    }

    void print(int x, int y, font::Font *font, display::TextAlign align, const char *text){
        this->elements.print(x, y, font, align, text);
    }

    void print(int x, int y, font::Font *font, const char *text){
        this->elements.print(x, y, font, text);
    }
    
    void printf(int x, int y, font::Font *font, Color color, Color background, display::TextAlign align, const char *format, ...)
        __attribute__((format(printf, 8, 9))){
        va_list arg;
        va_start(arg, format);
        this->elements.vprintf_(x, y, font, color, background, align, format, arg);
        va_end(arg);
    }
    
    void printf(int x, int y, font::Font *font, Color color, display::TextAlign align, const char *format, ...)
        __attribute__((format(printf, 7, 8))){
        va_list arg;
        va_start(arg, format);
        this->elements.vprintf_(x, y, font, color, display::COLOR_OFF, align, format, arg);
        va_end(arg);
    }
    
    void printf(int x, int y, font::Font *font, Color color, const char *format, ...) __attribute__((format(printf, 6, 7))){
        va_list arg;
        va_start(arg, format);
        this->elements.vprintf_(x, y, font, color, display::COLOR_OFF, display::TextAlign::TOP_LEFT, format, arg);
        va_end(arg);
    }
    
    void printf(int x, int y, font::Font *font, display::TextAlign align, const char *format, ...)
        __attribute__((format(printf, 6, 7))){
        va_list arg;
        va_start(arg, format);
        this->elements.vprintf_(x, y, font, display::COLOR_ON, display::COLOR_OFF, align, format, arg);
        va_end(arg);
    }
    
    void printf(int x, int y, font::Font *font, const char *format, ...) __attribute__((format(printf, 5, 6))){
        va_list arg;
        va_start(arg, format);
        this->elements.vprintf_(x, y, font, display::COLOR_ON, display::COLOR_OFF, display::TextAlign::TOP_LEFT, format, arg);
        va_end(arg);
    }

    void strftime(int x, int y, font::Font *font, Color color, display::TextAlign align, const char *format, ESPTime time) __attribute__((format(strftime, 7, 0))){
        this->elements.strftime(x, y, font, color, align, format, time);
    }

    void strftime(int x, int y, font::Font *font, Color color, const char *format, ESPTime time) __attribute__((format(strftime, 6, 0))){
        this->elements.strftime(x, y, font, color,format, time);
    }

    void strftime(int x, int y, font::Font *font, display::TextAlign align, const char *format, ESPTime time) __attribute__((format(strftime, 6, 0))){
        this->elements.strftime(x, y, font, align, format, time);
    }

    void strftime(int x, int y, font::Font *font, const char *format, ESPTime time) __attribute__((format(strftime, 5, 0))){
        this->elements.strftime(x, y, font, format, time);
    }

    void image(int x, int y, image::Image *image, Color color_on = display::COLOR_ON, Color color_off = display::COLOR_OFF){
        this->elements.image(x, y, image, color_on, color_off);
    }

    void image(int x, int y, image::Image *image, display::ImageAlign align, Color color_on = display::COLOR_ON, Color color_off = display::COLOR_OFF){
        this->elements.image(x, y, image, align, color_on, color_off);
    }
    
    elements::Elements<detail::WaveshareEPaper7P5InCProps> elements;
protected:
    uint32_t get_buffer_length_() override;
    int get_width_internal() override;
    int get_height_internal() override;

};


}  // namespace waveshare_epaper
}  // namespace esphome
