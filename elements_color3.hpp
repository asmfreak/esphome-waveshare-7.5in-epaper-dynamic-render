#pragma once
#include <esphome/core/color.h>
#include "elements_triplecolor.hpp"
#include "elements_math_utils.hpp"

namespace esphome {
namespace waveshare_epaper {
namespace elements {
template<typename T>
struct Unbound;

template<typename T=uint8_t, typename UL=int16_t>
struct AColor3{
    T red;
    T green;
    T blue;
    constexpr inline AColor3() ESPHOME_ALWAYS_INLINE : red(0), green(0), blue(0) {}  // NOLINT
    constexpr inline AColor3(T r, T g, T b) ESPHOME_ALWAYS_INLINE
        : red(r), green(g), blue(b)
    {}
    
    
    inline explicit AColor3(esphome::Color c) ESPHOME_ALWAYS_INLINE : red(c.red), green(c.green), blue(c.blue) {}  // NOLINT
    inline AColor3& operator=(const Color &rhs) {  // NOLINT
        *this = AColor3(rhs);
        return *this;
    }
    
    template<typename U, typename UUL>
    constexpr inline explicit AColor3(AColor3<U, UUL> c) : red(c.red), green(c.green), blue(c.blue) {}  // NOLINT
    template<typename U, typename UUL>    
    constexpr inline AColor3& operator=(const AColor3<U, UUL> &rhs) {  // NOLINT
        *this = AColor3(rhs);
        return *this;
    }
    
    inline bool operator==(const AColor3 &rhs) {  // NOLINT
        return \
                   this->red == rhs.red &&\
                     this->green == rhs.green &&\
                     this->blue == rhs.blue;
    }
    inline bool operator!=(const AColor3 &rhs) {  // NOLINT
        return \
                   this->red != rhs.red &&\
                     this->green != rhs.green &&\
                     this->blue != rhs.blue;
    }
    inline AColor3 operator~() const ESPHOME_ALWAYS_INLINE {
        return AColor3(255 - this->red, 255 - this->green, 255 - this->blue);
    }
    
    inline AColor3 operator+(const AColor3 &add) const ESPHOME_ALWAYS_INLINE {
        AColor3 ret;
        ret.red   = sat8<T, UL>(this->red   + add.red);
        ret.green = sat8<T, UL>(this->green + add.green);
        ret.blue  = sat8<T, UL>(this->blue  + add.blue);
        return ret;
    }
    inline AColor3 &operator+=(const AColor3 &add) ESPHOME_ALWAYS_INLINE { return *this = (*this) + add; }
    inline AColor3 operator+(T add) const ESPHOME_ALWAYS_INLINE { return (*this) + AColor3(add, add, add); }
    inline AColor3 &operator+=(T add) ESPHOME_ALWAYS_INLINE { return *this = (*this) + add; }
    
    inline AColor3 operator-(const AColor3 &subtract) const ESPHOME_ALWAYS_INLINE {
        AColor3 ret;
        ret.red   = sat8<T, UL>(this->red   - subtract.red);
        ret.green = sat8<T, UL>(this->green - subtract.green);
        ret.blue  = sat8<T, UL>(this->blue  - subtract.blue);
        return ret;
    }
    inline AColor3 &operator-=(const AColor3 &subtract) ESPHOME_ALWAYS_INLINE { return *this = (*this) - subtract; }
    inline AColor3 operator-(T subtract) const ESPHOME_ALWAYS_INLINE {
        return (*this) - AColor3(subtract, subtract, subtract);
    }
    inline AColor3 &operator-=(const T &subtract) ESPHOME_ALWAYS_INLINE { return *this = (*this) - subtract; }
    
    inline AColor3 operator*(T scale) const ESPHOME_ALWAYS_INLINE {
        return AColor3(
            esp_scale8<T, UL>(this->red, scale),
            esp_scale8<T, UL>(this->green, scale),
            esp_scale8<T, UL>(this->blue, scale));
    }
    inline AColor3 &operator*=(T scale) ESPHOME_ALWAYS_INLINE {
        this->red = esp_scale8<T, UL>(this->red, scale);
        this->green = esp_scale8<T, UL>(this->green, scale);
        this->blue = esp_scale8<T, UL>(this->blue, scale);
        return *this;
    }
    inline AColor3 operator*(const AColor3 &scale) const ESPHOME_ALWAYS_INLINE {
        return AColor3(
            esp_scale8<T, UL>(this->red, scale.red),
            esp_scale8<T, UL>(this->green, scale.green),
            esp_scale8<T, UL>(this->blue, scale.blue));
    }
    inline AColor3 &operator*=(const AColor3 &scale) ESPHOME_ALWAYS_INLINE {
        this->red = esp_scale8<T, UL>(this->red, scale.red);
        this->green = esp_scale8<T, UL>(this->green, scale.green);
        this->blue = esp_scale8<T, UL>(this->blue, scale.blue);
        return *this;
    }
    
    inline AColor3 operator/(T scale) const ESPHOME_ALWAYS_INLINE {
        return AColor3(
            esp_div8<T, UL>(this->red, scale),
            esp_div8<T, UL>(this->green, scale),
            esp_div8<T, UL>(this->blue, scale));
    }
    inline AColor3 &operator/=(T scale) ESPHOME_ALWAYS_INLINE {
        this->red = esp_div8<T, UL>(this->red, scale);
        this->green = esp_div8<T, UL>(this->green, scale);
        this->blue = esp_div8<T, UL>(this->blue, scale);
        return *this;
    }
    inline AColor3 operator/(const AColor3 &scale) const ESPHOME_ALWAYS_INLINE {
        return AColor3(
            esp_div8<T, UL>(this->red, scale.red),
            esp_div8<T, UL>(this->green, scale.green),
            esp_div8<T, UL>(this->blue, scale.blue));
    }
    inline AColor3 &operator/=(const AColor3 &scale) ESPHOME_ALWAYS_INLINE {
        this->red = esp_div8<T, UL>(this->red, scale.red);
        this->green = esp_div8<T, UL>(this->green, scale.green);
        this->blue = esp_div8<T, UL>(this->blue, scale.blue);
        return *this;
    }
    
    
    inline AColor3 operator*(const Unbound<AColor3> &scale) const ESPHOME_ALWAYS_INLINE {
        return AColor3(
            this->red * scale.color.red,
            this->green * scale.color.green,
            this->blue * scale.color.blue);
    }
    inline AColor3 &operator*=(const Unbound<AColor3> &scale) ESPHOME_ALWAYS_INLINE {
        return *this = (*this) * scale;
    }
    inline AColor3 operator*(Unbound<T> scale) const ESPHOME_ALWAYS_INLINE {
        return (*this) * Unbound<AColor3>{AColor3(scale.color,scale.color,scale.color)};
    }
    inline AColor3 &operator*=(Unbound<T> scale) ESPHOME_ALWAYS_INLINE {
        return *this = (*this) * scale;
    }
    
    inline AColor3 operator/(const Unbound<AColor3> &scale) const ESPHOME_ALWAYS_INLINE {
        auto divUnbound = [](T l, T r) -> T{
            if (r == 0) {
                return inf(l);
            }
            return l / r;
        };
        return AColor3(
            divUnbound(this->red, scale.color.red),
            divUnbound(this->green, scale.color.green),
            divUnbound(this->blue, scale.color.blue));
    }
    inline AColor3 &operator/=(const Unbound<AColor3> &scale) ESPHOME_ALWAYS_INLINE {
        return *this = (*this) * scale;
    }
    inline AColor3 operator/(Unbound<T> scale) const ESPHOME_ALWAYS_INLINE {
        return (*this) * AColor3(scale,scale,scale);
    }
    inline AColor3 &operator/=(Unbound<T> scale) ESPHOME_ALWAYS_INLINE {
        return *this = (*this) * scale;
    }
    
    inline AColor3 operator+(const Unbound<AColor3> &add) const ESPHOME_ALWAYS_INLINE {
        AColor3 ret;
        ret.red   = this->red   + add.color.red;
        ret.green = this->green + add.color.green;
        ret.blue  = this->blue  + add.color.blue;
        return ret;
    }
    inline AColor3 &operator+=(const Unbound<AColor3> &add) ESPHOME_ALWAYS_INLINE { return *this = (*this) + add; }
    inline AColor3 operator+(Unbound<T> add) const ESPHOME_ALWAYS_INLINE {
        return (*this) + Unbound<AColor3>{AColor3(add.color, add.color, add.color)};
    }
    inline AColor3 &operator+=(Unbound<T> add) ESPHOME_ALWAYS_INLINE { return *this = (*this) + add; }
    inline AColor3 operator-(const Unbound<AColor3> &subtract) const ESPHOME_ALWAYS_INLINE {
        AColor3 ret;
        ret.red   = this->red   - subtract.color.red;
        ret.green = this->green - subtract.color.green;
        ret.blue  = this->blue  - subtract.color.blue;
        return ret;
    }
    
    inline AColor3 &operator-=(const Unbound<AColor3> &subtract) ESPHOME_ALWAYS_INLINE { return *this = (*this) - subtract; }
    inline AColor3 operator-(Unbound<T> subtract) const ESPHOME_ALWAYS_INLINE {
        return (*this) - Unbound(AColor3(subtract.color, subtract.color, subtract.color));
    }
    inline AColor3 &operator-=(Unbound<T> add) ESPHOME_ALWAYS_INLINE { return *this = (*this) - add; }
};

template<typename T>
struct Unbound{
    T color;
};

template<typename T>
Unbound<T> unb(T v){
    return Unbound<T>{v};
}

using Color3 = AColor3<>;
using Color3S = AColor3<int8_t, int16_t>;
using Color3S_16 = AColor3<int16_t, int32_t>;
using Color3F = AColor3<float,float>;

// extern Color3 
TripleColor col2bin(Color3 c);
Color3 col2pallete(Color3F c);

} // namespace esphome
} // namespace waveshare_epaper
} // namespace elements
