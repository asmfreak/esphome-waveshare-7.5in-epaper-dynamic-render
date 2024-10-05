#pragma once

#include "esphome/core/color.h"
#ifndef IN_EMULATION
#include "esphome/core/hal.h"
#endif // ndef IN_EMULATION
#include "esphome/components/font/font.h"
#include "esphome/components/image/image.h"

#include <algorithm>

#include <map>
#include <variant>
#include <vector>
#include <cstdarg>
#include <cmath>
#include "rtraits.hpp"

#include "elements_color3.hpp"
#include "elements_geometric.hpp"

namespace esphome {
namespace waveshare_epaper {
namespace elements {

Trait2(
    Elemental,
    (boundingBox, Rect2D, (), const),
    (pixAt, esphome::optional<Color3>, (int x, int y), const)
)
    
    
class LineElement{
    std::vector<Point2D> vertexes;
    Color3 c;
    Rect2D bb;
public:
    LineElement(Color3 color, std::vector<Point2D> il):vertexes(std::move(il)), c(color), bb(calculateBoundingBox()){
    }
    esphome::optional<Color3> pixAt(int x, int y) const {
        if(vertexes.size() < 2){
            return esphome::nullopt;
        }
        const Point2D p{x,y};
        if(!bb.has(p)){
            return esphome::nullopt;
        }
        auto prev = vertexes.cbegin();
        const auto e = vertexes.cend();
        float prevd = ((*prev) - p).len();
        for(auto i=prev+1; i!=e; ++i){
            auto ld = ((*prev) - (*i)).len();
            auto cd = ((*i) - p).len();
            
            if((prevd + cd - ld) < 0.01){
                return c;
            }
            prevd = cd;
            prev = i;
        }
        return esphome::nullopt;
    }
    Rect2D boundingBox() const {
        return bb;
    }
private:
    Rect2D calculateBoundingBox() const{
        if(vertexes.empty()){
            return Rect2D{};
        }
        auto mmx = std::minmax_element(
            std::begin(vertexes),std::end(vertexes),
            [](const Point2D&l, const Point2D&r)->bool{ return l.x < r.x; });
        auto mmy = std::minmax_element(
            std::begin(vertexes),std::end(vertexes),
            [](const Point2D&l, const Point2D&r)->bool{ return l.y < r.y; });
        
        return Rect2D{
            Point2D{mmx.first->x, mmy.first->y},
            Point2D{mmx.second->x, mmy.second->y}
        };
    }
};

class RectElement {
    Rect2D rect;
    esphome::optional<Color3> borders;
    esphome::optional<Color3> fill;
public:
    RectElement(Rect2D r, esphome::optional<Color3> b, esphome::optional<Color3> f)
        :rect(r), borders(b), fill(f)
    {}
    RectElement(const RectElement &) = default;
    RectElement(RectElement &&) = default;
    RectElement &operator=(const RectElement &) = default;
    RectElement &operator=(RectElement &&) = default;
    
    esphome::optional<Color3> pixAt(int x, int y) const {
        const auto p = Point2D{x,y};
        if(not rect.has(p)){
            return esphome::nullopt;
        }
        if(borders.has_value()){
            const bool lb = x == rect.tl.x;
            const bool rb = x == rect.br.x;
            const bool tb = y == rect.tl.y;
            const bool bb = y == rect.br.y;
            if(lb || rb || tb || bb){
                return borders;
            }
        }
        return fill;
    }
    
    Rect2D boundingBox() const{
        return rect;
    }
};


class TriangleElement {
    Triangle2D tri;
    Color3 fill;
public:
    TriangleElement(Triangle2D r, Color3 f)
        :tri(r), fill(f)
    {}
    TriangleElement(const TriangleElement &) = default;
    TriangleElement(TriangleElement &&) = default;
    TriangleElement &operator=(const TriangleElement &) = default;
    TriangleElement &operator=(TriangleElement &&) = default;
    
    esphome::optional<Color3> pixAt(int x, int y) const {
        const auto p = Point2D{x,y};
        if(not tri.has(p)){
            return esphome::nullopt;
        }
        return fill;
    }
    
    Rect2D boundingBox() const{
        return tri.boundingBox();
    }
};

class CircleElement {
    Circle2D tri;
    Color3 fill;
    display::RegularPolygonDrawing drawing;
public:
    CircleElement(Circle2D r, Color3 f, display::RegularPolygonDrawing d)
        :tri(r), fill(f), drawing(d)
    {}
    CircleElement(const CircleElement &) = default;
    CircleElement(CircleElement &&) = default;
    CircleElement &operator=(const CircleElement &) = default;
    CircleElement &operator=(CircleElement &&) = default;
    
    esphome::optional<Color3> pixAt(int x, int y) const {
        const auto p = Point2D{x,y};
        const auto has = tri.has(p);
        if(has == HasPoint::Outside){
            return esphome::nullopt;
        }
        if(has == HasPoint::OnBorder || drawing == display::DRAWING_FILLED){
            return fill;
        }
        return esphome::nullopt;
    }
    
    Rect2D boundingBox() const{
        return tri.boundingBox();
    }
};

class LinearGradient{
    Rect2D rect;
    Color3 start;
    Color3 end;
    
public:
    explicit LinearGradient(Rect2D r, Color3 s, Color3 e)
        :rect(r), start(s), end(e)
    {}
    LinearGradient(const LinearGradient &) = default;
    LinearGradient(LinearGradient &&) = default;
    LinearGradient &operator=(const LinearGradient &) = default;
    LinearGradient &operator=(LinearGradient &&) = default;
    
    esphome::optional<Color3> pixAt(int x, int y) const {
        const auto p = Point2D{x,y};
        if(not rect.has(p)){
            return esphome::nullopt;
        }
        const auto maxWidth = rect.br.x - rect.tl.x;
        if(maxWidth == 0){
            return esphome::nullopt;
        }
        const auto currentWidth = p.x - rect.tl.x;
        const auto ret = Color3(
            Color3F(start) + ((Color3F(end) - Color3F(start)) * (float(currentWidth) / maxWidth))
        );
        return ret;
    }
    
    Rect2D boundingBox() const{
        return rect;
    }
};

class Texture{
    Rect2D rect;
    std::vector<Color3> pixels;
    
public:
    Texture():rect(),pixels(){}
    template<typename F>
    Texture(Point2D pos, Point2D size, F&& f):rect{pos, pos + size - Point2D{1,1}}, pixels(){
        pixels.reserve(size.x *size.y);
        for(int y=0; y < size.y; ++y){
            for(int x=0; x < size.x; ++x){
                pixels.emplace_back(f(x,y));
            }
        }
    }
    Texture(const Texture &) = default;
    Texture(Texture &&) = default;
    Texture &operator=(const Texture &) = default;
    Texture &operator=(Texture &&) = default;
    
    esphome::optional<Color3> pixAt(int x, int y) const {
        auto p = Point2D{x,y};
        if (not rect.has(p)){
            return esphome::nullopt;
        }
        auto i = p - rect.tl;
        return pixels.at(i.x + rect.width() * i.y);
    }
    
    Rect2D boundingBox() const {
        return rect;
    }
};

template<typename F>
class TextureFunction{
    Rect2D rect;
    F func;
    
public:
    TextureFunction(Point2D pos, Point2D size, F f):rect{pos, pos + size}, func(std::move(f)){
    }
    TextureFunction(const TextureFunction &) = default;
    TextureFunction(TextureFunction &&) = default;
    TextureFunction &operator=(const TextureFunction &) = default;
    TextureFunction &operator=(TextureFunction &&) = default;
    
    esphome::optional<Color3> pixAt(int x, int y) const {
        auto p = Point2D{x,y};
        if (not rect.has(p)){
            return esphome::nullopt;
        }
        auto i = p - rect.tl;
        return func(i.x, i.y);
    }
    
    Rect2D boundingBox() const {
        return rect;
    }
};

template<typename F>
TextureFunction(Point2D pos, Point2D size, F f) -> TextureFunction<F>;

struct FontGlyph{
    const font::Glyph* glyph;
    int bpp;
};


struct Glyph{
    Rect2D rect;
    FontGlyph g;
    Color3 fg;
    esphome::optional<Color3> bg;
    esphome::optional<Color3F> diff;
    uint8_t bpp_max;
public:
    Glyph(Point2D pos, Point2D size, FontGlyph glyph, Color3 color, esphome::optional<Color3> background):
        rect{pos, pos + size - Point2D{1,1}},
        g(glyph), fg(color), bg(background), diff(esphome::nullopt)
    {
        bpp_max = (1 << g.bpp) - 1;
        if(bg.has_value()){
            diff = Color3F(color) - Color3F(bg.value());
        }
    }
    esphome::optional<Color3> pixAt(int x, int y) const {
        const auto p = Point2D{x,y};
        if(not rect.has(p)){
            return esphome::nullopt;
        }
        const auto gd = g.glyph->get_glyph_data();
        const auto glyphTL = rect.tl + Point2D{gd->offset_x, gd->offset_y};
        Rect2D glyphRect = {
            glyphTL,
            glyphTL + Point2D{gd->width, gd->height} - Point2D{1, 1}
        };
        if (not glyphRect.has(p)){
            return bg;
        }
        const auto i = p - glyphTL;
        const auto dataposBits = (i.x + i.y * gd->width) * g.bpp;
        
        const auto dataposBytesBits = std::div(dataposBits, 8);
        const uint8_t *data = gd->data + dataposBytesBits.quot;
        uint8_t bitmask = 0x80 >> dataposBytesBits.rem;
        auto pixel_data = progmem_read_byte(data++);
        uint8_t pixel = 0;
        for (int bit_num = 0; bit_num != g.bpp; bit_num++) {
            if (bitmask == 0) {
                pixel_data = progmem_read_byte(data++);
                bitmask = 0x80;
            }
            pixel <<= 1;
            if ((pixel_data & bitmask) != 0)
                pixel |= 1;
            bitmask >>= 1;
        }
        
        if (pixel == bpp_max) {
            return fg;
        } else if (pixel != 0) {
            if(bg.has_value()){
                auto on = (float) pixel / (float) bpp_max;
                auto blended = Color3F(diff.value()) * unb(on) + unb(Color3F(bg.value()));
                return Color3(blended);
            }
        }else{
            return bg;
        }
        return esphome::nullopt;
    }
    Rect2D boundingBox() const {
        return rect;
    }
};

class SparseTexture{
    std::map<Point2D, Color3> m;

public:
    SparseTexture() = default;
    SparseTexture(const SparseTexture &) = default;
    SparseTexture(SparseTexture &&) = default;
    SparseTexture &operator=(const SparseTexture &) = default;
    SparseTexture &operator=(SparseTexture &&) = default;
    
    void insert(Point2D pos, Color3 color){
        m.insert(std::pair<Point2D, Color3>(pos, color));
    }

    esphome::optional<Color3> pixAt(int x, int y) const {
        auto iter = m.find(Point2D{x, y});
        if (iter != m.end()) {
            return esphome::optional<Color3>{iter->second};
        }
        return esphome::nullopt;
    }
    
    Rect2D boundingBox() const {
        if(m.size() > 0){
            return Rect2D{
                m.begin()->first,
                m.rbegin()->first
            };
        }
        return Rect2D{};
    }
};

using Element = std::variant<Texture, SparseTexture, LinearGradient>;

namespace detail{
template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin (reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end (reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse (T&& iterable) { return { iterable }; }
}

template<typename Base>
class Elements{
    std::vector<Elemental_Owning> els;
    Color3 bg;
    Color3S_16 dither[Base::static_width_()*2];
#ifdef IN_EMULATION
    Color3 origR[Base::static_width_()*2];
#endif//def IN_EMULATION
public:
    Elements():els(), bg(0,0,0){
    }
    
    void fill(Color3 bg){
        this->bg = bg;
    }
    
    template<typename T, typename... A>
    T* append_element(A... e){
        els.emplace_back(makeElemental<T>(std::forward<A>(e)...));
        return trait_cast<T>(els.back());
    }
    
    template<template<typename...>typename T, typename... TP, typename... A>
    void append_element(A... e){
        T t{std::forward<A>(e)...};
        els.emplace_back(makeElemental<decltype(t)>(std::move(t)));
        // return trait_cast<T>(els.back());
    }

    Color3 pixAt(int x, int y) const{
        esphome::optional<Color3> ret{esphome::nullopt};
        auto elsR =  detail::reverse(els);
        for(const auto& el:elsR){
            ret = el.pixAt(x,y);
            if(ret.has_value()){
                return ret.value();
            }
        }
        return bg;
    }

    template<typename F>
    void render(F&& f){
        size_t dither_current=0;
        size_t dither_next=Base::static_width_();
        {
            size_t x = 0;
            std::generate_n(std::begin(dither)+dither_current, Base::static_width_(), [this, &x]() ->Color3{
                return Color3{pixAt(x++,0)};
            });
        }
#ifdef IN_EMULATION
        std::copy(std::begin(dither), std::end(dither), std::begin(origR));
#endif//def IN_EMULATION
        for(size_t y=0; y < Base::static_height_(); ++y){
            size_t x = 0;
            std::generate_n(std::begin(dither)+dither_next, Base::static_width_(), [this, &x, y]() ->Color3{
                return Color3{pixAt(x++,y)};
            });
#ifdef IN_EMULATION
            std::copy_n(std::begin(dither)+dither_next, Base::static_width_(), std::begin(origR)+dither_next);
#endif//def IN_EMULATION
            for(size_t x=0; x < Base::static_width_(); ++x){
                const auto currentPix = dither[x+dither_current];
                const auto pallettePix = col2pallete(Color3F{currentPix});
                const auto quantError = Color3F(currentPix) - unb(Color3F(pallettePix));
                if(x == 0){
                    dither[x + 1 + dither_current   ] += Color3S_16(quantError * unb<float>(7./32.));
                    dither[x     + dither_next      ] += Color3S_16(quantError * unb<float>(7./32.));
                    dither[x + 1 + dither_next      ] += Color3S_16(quantError * unb<float>(2./32.));
                }else if(x == Base::static_width_()-1){
                    dither[x - 1 + dither_next      ] += Color3S_16(quantError * unb<float>(7./32.));
                    dither[x     + dither_next      ] += Color3S_16(quantError * unb<float>(9./32.));
                }else {
                    dither[x + 1 + dither_current   ] += Color3S_16(quantError * unb<float>(7./32.));
                    dither[x - 1 + dither_next      ] += Color3S_16(quantError * unb<float>(3./32.));
                    dither[x     + dither_next      ] += Color3S_16(quantError * unb<float>(5./32.));
                    dither[x + 1 + dither_next      ] += Color3S_16(quantError * unb<float>(1./32.));
                }
                f(
                    x
                    , y
                    , pallettePix
#ifdef IN_EMULATION
                    , Color3(origR[x+dither_current])
                    , Color3(currentPix)
#endif//def IN_EMULATION                    
                );
            }
            std::swap(dither_current, dither_next);
        }
    }

    void clear(){
        els.clear();
    }
    void draw_pixel_at(int x, int y){
        draw_pixel_at(x, y, display::COLOR_ON);
    }
    void draw_pixel_at(int x, int y, Color color){
        SparseTexture *sp;
        if(els.empty()){
            sp = append_element<SparseTexture>();
        }
        sp->insert(Point2D{x,y}, Color3{color});
    }
    
    void draw_pixels_at(int x_start, int y_start, int w, int h, const uint8_t *ptr, display::ColorOrder order,
                        display::ColorBitness bitness, bool big_endian, int x_offset, int y_offset, int x_pad){}
    
    void line(int x1, int y1, int x2, int y2, Color color = display::COLOR_ON){
        append_element<LineElement>(
            Color3{color}, std::vector<Point2D>{ Point2D{x1,y1}, Point2D{x2, y2} }
        );
    }
    
    void line_at_angle(int x, int y, int angle, int length, Color color = display::COLOR_ON);
    
    void line_at_angle(int x, int y, int angle, int start_radius, int stop_radius, Color color = display::COLOR_ON);
    
    void horizontal_line(int x, int y, int width, Color color = display::COLOR_ON){
        append_element<LineElement>(
            Color3{color}, std::vector<Point2D>{ Point2D{x,y}, Point2D{x+width, y} }
        );
    }
    
    void vertical_line(int x, int y, int height, Color color = display::COLOR_ON){
        append_element<LineElement>(
            Color3{color}, std::vector<Point2D>{ Point2D{x,y}, Point2D{x, y + height} }
        );
    }
    
    void rectangle(int x1, int y1, int width, int height, Color color = display::COLOR_ON){
        const Point2D tl{x1, y1};
        
        append_element<RectElement>(
            Rect2D{tl, tl + Point2D{width, height} - Point2D{1,1}}, Color3{color}, esphome::nullopt
        );
    }
    
    void filled_rectangle(int x1, int y1, int width, int height, Color color = display::COLOR_ON){
        const Point2D tl{x1, y1};
        
        append_element<RectElement>(
            Rect2D{tl, tl + Point2D{width, height} - Point2D{1,1}}, esphome::nullopt, Color3{color}
        );
    }
    
    void circle(int center_x, int center_y, int radius, Color color = display::COLOR_ON){
        append_element<CircleElement>(
            Circle2D{{center_x,center_y}, float(radius)}, Color3{color},  display::DRAWING_OUTLINE
            );
    }
    
    void filled_circle(int center_x, int center_y, int radius, Color color = display::COLOR_ON){
        append_element<CircleElement>(
            Circle2D{{center_x,center_y}, float(radius)}, Color3{color},  display::DRAWING_FILLED
            );
    }
    
    void triangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color = display::COLOR_ON){
        append_element<LineElement>(
            Color3{color}, std::vector<Point2D>{ Point2D{x1,y1}, Point2D{x2, y2}, Point2D{x3, y3}, Point2D{x1,y1} }
        );
    }
    
    void filled_triangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color = display::COLOR_ON){
        append_element<TriangleElement>(
            Triangle2D{{x1,y1}, {x2,y2}, {x3,y3}}, Color3{color}
            );
    }
    
    void get_regular_polygon_vertex(int vertex_id, int *vertex_x, int *vertex_y, int center_x, int center_y, int radius,
                                    int edges, display::RegularPolygonVariation variation = display::VARIATION_POINTY_TOP,
                                    float rotation_degrees = display::ROTATION_0_DEGREES){
        if (edges >= 2) {
            // Given the orientation of the display component, an angle is measured clockwise from the x axis.
            // For a regular polygon, the human reference would be the top of the polygon,
            // hence we rotate the shape by 270° to orient the polygon up.
            rotation_degrees += display::ROTATION_270_DEGREES;
            // Convert the rotation to radians, easier to use in trigonometrical calculations
            float rotation_radians = rotation_degrees * PI / 180;
            // A pointy top variation means the first vertex of the polygon is at the top center of the shape, this requires no
            // additional rotation of the shape.
            // A flat top variation means the first point of the polygon has to be rotated so that the first edge is horizontal,
            // this requires to rotate the shape by π/edges radians counter-clockwise so that the first point is located on the
            // left side of the first horizontal edge.
            rotation_radians -= (variation == display::VARIATION_FLAT_TOP) ? PI / edges : 0.0;
            
            float vertex_angle = ((float) vertex_id) / edges * 2 * PI + rotation_radians;
            *vertex_x = (int) round(cos(vertex_angle) * radius) + center_x;
            *vertex_y = (int) round(sin(vertex_angle) * radius) + center_y;
        }
    }
    
    void regular_polygon(
            int x, int y, int radius, int edges,
            const display::RegularPolygonVariation variation = display::VARIATION_POINTY_TOP,
            const float rotation_degrees = display::ROTATION_0_DEGREES,
            const Color color = display::COLOR_ON,
            const display::RegularPolygonDrawing drawing  = display::DRAWING_OUTLINE){
        if (edges >= 2) {
            std::vector<Point2D> pts;
            int previous_vertex_x=0, previous_vertex_y=0;
            for (int current_vertex_id = 0; current_vertex_id <= edges; current_vertex_id++) {
                int current_vertex_x, current_vertex_y;
                get_regular_polygon_vertex(current_vertex_id, &current_vertex_x, &current_vertex_y, x, y, radius, edges,
                                           variation, rotation_degrees);
                pts.push_back(Point2D{current_vertex_x, current_vertex_y});
                if (current_vertex_id !=0 && drawing == display::DRAWING_FILLED) {
                    this->filled_triangle(x, y, previous_vertex_x, previous_vertex_y, current_vertex_x, current_vertex_y, color);
                }
                previous_vertex_x = current_vertex_x;
                previous_vertex_y = current_vertex_y;
            }
            if (drawing == display::DRAWING_OUTLINE) {
                append_element<LineElement>(Color3{color}, std::move(pts));
            }
        }
    }
    void regular_polygon(int x, int y, int radius, int edges, display::RegularPolygonVariation variation, Color color,
                         display::RegularPolygonDrawing drawing = display::DRAWING_OUTLINE){
        regular_polygon(x, y, radius, edges, variation, display::ROTATION_0_DEGREES, color, drawing);
    }
    void regular_polygon(int x, int y, int radius, int edges, Color color,
                         display::RegularPolygonDrawing drawing = display::DRAWING_OUTLINE){
        regular_polygon(x, y, radius, edges, display::VARIATION_POINTY_TOP, display::ROTATION_0_DEGREES, color, drawing);
    }
    
    void filled_regular_polygon(int x, int y, int radius, int edges,
                                display::RegularPolygonVariation variation = display::VARIATION_POINTY_TOP,
                                float rotation_degrees = display::ROTATION_0_DEGREES, Color color = display::COLOR_ON){
        regular_polygon(x, y, radius, edges, variation, rotation_degrees, color, display::DRAWING_FILLED);
    }
    void filled_regular_polygon(int x, int y, int radius, int edges, display::RegularPolygonVariation variation, Color color){
        regular_polygon(x, y, radius, edges, variation, display::ROTATION_0_DEGREES, color, display::DRAWING_FILLED);
    }
    void filled_regular_polygon(int x, int y, int radius, int edges, Color color){
        regular_polygon(x, y, radius, edges, display::VARIATION_POINTY_TOP, display::ROTATION_0_DEGREES, color, display::DRAWING_FILLED);
    }
    
    void print(int xp, int yp, font::Font *font, Color color, display::TextAlign align, const char *text,
               esphome::optional<Color> background = display::COLOR_OFF){
        esphome::optional<Color3> bg{esphome::nullopt};
        if(background.has_value()){
            bg = Color3{background.value()};
        }
        int width;
        int height = font->get_height();
        int baseline = font->get_baseline();
        int i = 0;
        int min_x = 0;
        bool has_char = false;
        int x = 0;
        const auto& glyphs = font->get_glyphs();
        struct GlyphArgs{
            Point2D pos;
            Point2D size;
            FontGlyph glyph;
            Color3 color;
            esphome::optional<Color3> background;
            GlyphArgs(Point2D p,Point2D s,FontGlyph g,Color3 c,esphome::optional<Color3> b):pos(p),size(s),glyph(g),color(c),background(b){}
        };
        std::vector<GlyphArgs> textGlyphs;
        while (text[i] != '\0') {
            int match_length;
            int glyph_n = font->match_next_glyph((const uint8_t *) text + i, &match_length);
            if (glyph_n < 0) {
                // Unknown char, skip
                if (!glyphs.empty())
                    x += glyphs[0].get_glyph_data()->width;
                i++;
                continue;
            }
            
            const font::Glyph &glyph = glyphs[glyph_n];
            const auto glyphData = glyph.get_glyph_data();
            if (!has_char) {
                min_x = glyphData->offset_x;
            } else {
                min_x = std::min(min_x, x + glyphData->offset_x);
            }
            textGlyphs.emplace_back(Point2D{x, 0}, Point2D{glyphData->width, height}, FontGlyph{&glyph, font->get_bpp()}, Color3{color}, bg);
            x += glyphData->width + glyphData->offset_x;
            i += match_length;
            has_char = true;
        }
        width = x - min_x;
        
        const auto x_align = display::TextAlign(int(align) & 0x18);
        const auto y_align = display::TextAlign(int(align) & 0x07);
        
        switch (x_align) {
        case display::TextAlign::RIGHT:
            xp = xp - width;
            break;
        case display::TextAlign::CENTER_HORIZONTAL:
            xp = xp - (width) / 2;
            break;
        case display::TextAlign::LEFT:
        default:
            break;
        }
        
        switch (y_align) {
        case display::TextAlign::BOTTOM:
            yp = yp - height;
            break;
        case display::TextAlign::BASELINE:
            yp = yp - baseline;
            break;
        case display::TextAlign::CENTER_VERTICAL:
            yp = yp - (height) / 2;
            break;
        case display::TextAlign::TOP:
        default:
            break;
        }
        
        for(auto& i:textGlyphs){
            append_element<Glyph>(i.pos + Point2D{xp, yp}, i.size, i.glyph, i.color, i.background);
        }
    }
    
    void print(int x, int y, font::Font *font, Color color, const char *text, Color background = display::COLOR_OFF){
        this->print(x, y, font, color, display::TextAlign::TOP_LEFT, text, background);
    }
    
    void print(int x, int y, font::Font *font, display::TextAlign align, const char *text){
        this->print(x, y, font, display::COLOR_ON, align, text);
    }
    
    void print(int x, int y, font::Font *font, const char *text){
        this->print(x, y, font, display::COLOR_ON,  display::TextAlign::TOP_LEFT, text);
    }
    
    void vprintf_(int x, int y, font::Font *font, Color color, Color background, display::TextAlign align, const char *format,
                           va_list arg) {
        char buffer[256];
        int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
        if (ret){
            this->print(x, y, font, color, align, buffer, background);
        }
    }

    void printf(int x, int y, font::Font *font, Color color, Color background, display::TextAlign align, const char *format, ...)
        __attribute__((format(printf, 8, 9))){
        va_list arg;
        va_start(arg, format);
        this->vprintf_(x, y, font, color, background, align, format, arg);
        va_end(arg);
    }
    
    void printf(int x, int y, font::Font *font, Color color, display::TextAlign align, const char *format, ...)
        __attribute__((format(printf, 7, 8))){
        va_list arg;
        va_start(arg, format);
        this->vprintf_(x, y, font, color, display::COLOR_OFF, align, format, arg);
        va_end(arg);
    }
    
    void printf(int x, int y, font::Font *font, Color color, const char *format, ...) __attribute__((format(printf, 6, 7))){
        va_list arg;
        va_start(arg, format);
        this->vprintf_(x, y, font, color, display::COLOR_OFF, display::TextAlign::TOP_LEFT, format, arg);
        va_end(arg);
    }
    
    void printf(int x, int y, font::Font *font, display::TextAlign align, const char *format, ...)
        __attribute__((format(printf, 6, 7))){
        va_list arg;
        va_start(arg, format);
        this->vprintf_(x, y, font, display::COLOR_ON, display::COLOR_OFF, align, format, arg);
        va_end(arg);
    }
    
    void printf(int x, int y, font::Font *font, const char *format, ...) __attribute__((format(printf, 5, 6))){
        va_list arg;
        va_start(arg, format);
        this->vprintf_(x, y, font, display::COLOR_ON, display::COLOR_OFF, display::TextAlign::TOP_LEFT, format, arg);
        va_end(arg);
    }
    
    void strftime(int x, int y, font::Font *font, Color color, display::TextAlign align, const char *format, ESPTime time)
        __attribute__((format(strftime, 7, 0))){
        char buffer[64];
        size_t ret = time.strftime(buffer, sizeof(buffer), format);
        if (ret > 0)
            this->print(x, y, font, color, align, buffer);
    }
    
    void strftime(int x, int y, font::Font *font, Color color, const char *format, ESPTime time)
        __attribute__((format(strftime, 6, 0))){
        this->strftime(x, y, font, color, display::TextAlign::TOP_LEFT, format, time);
    }
    
    void strftime(int x, int y, font::Font *font, display::TextAlign align, const char *format, ESPTime time)
        __attribute__((format(strftime, 6, 0))){
        this->strftime(x, y, font, display::COLOR_ON, align, format, time);
    }
    
    void strftime(int x, int y, font::Font *font, const char *format, ESPTime time) __attribute__((format(strftime, 5, 0))){
        this->strftime(x, y, font, display::COLOR_ON, display::TextAlign::TOP_LEFT, format, time);
    }
    
    void image(int x, int y, image::Image *image, Color color_on = display::COLOR_ON, Color color_off = display::COLOR_OFF){
        this->image(x, y, image, display::ImageAlign::TOP_LEFT, color_on, color_off);
    }
    
    void image(int x, int y, image::Image *image, display::ImageAlign align, Color color_on = display::COLOR_ON, Color color_off = display::COLOR_OFF){
        auto x_align = display::ImageAlign(int(align) & (int(display::ImageAlign::HORIZONTAL_ALIGNMENT)));
        auto y_align = display::ImageAlign(int(align) & (int(display::ImageAlign::VERTICAL_ALIGNMENT)));
        
        switch (x_align) {
        case display::ImageAlign::RIGHT:
            x -= image->get_width();
            break;
        case display::ImageAlign::CENTER_HORIZONTAL:
            x -= image->get_width() / 2;
            break;
        case display::ImageAlign::LEFT:
        default:
            break;
        }
        
        switch (y_align) {
        case display::ImageAlign::BOTTOM:
            y -= image->get_height();
            break;
        case display::ImageAlign::CENTER_VERTICAL:
            y -= image->get_height() / 2;
            break;
        case display::ImageAlign::TOP:
        default:
            break;
        }
        append_element<TextureFunction>(
            Point2D{x,y}, Point2D{image->get_width(), image->get_height()},
            [image, color_on, color_off](int x, int y)->Color3{
                return Color3(image->get_pixel(x, y, color_on, color_off));
            });
    }
    
#ifdef USE_QR_CODE
    void Display::qr_code(int x, int y, qr_code::QrCode *qr_code, Color color_on, int scale) {
        qr_code->draw(this, x, y, color_on, scale);
    }
#endif  // USE_QR_CODE
};

}  // namespace elements
}  // namespace waveshare_epaper
}  // namespace esphome
