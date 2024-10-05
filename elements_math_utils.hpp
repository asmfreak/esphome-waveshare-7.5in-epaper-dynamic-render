#pragma once
#include <cstdint>
#include <limits>

namespace esphome {
namespace waveshare_epaper {
namespace elements {

template<typename T>
T inf(const T& s){
    if constexpr (std::numeric_limits<T>::has_infinity){
        if (s > 0){
            return std::numeric_limits<T>::infinity();
        }else{
            return -std::numeric_limits<T>::infinity();
        }
    }
    if (s > 0){
        return std::numeric_limits<T>::max();
    }else{
        return -std::numeric_limits<T>::min();
    }
}

template<typename T, typename UL>
T sat8(UL t){
    if(int16_t(t) > 255){
        return 255;
    }
    if(int16_t(t) < -255){
        return -255;
    }
    return t;
}

template<typename T, typename UL>
T abs8(UL t){
    if(t < 0){
        return -t;
    }
    return t;
}

template<typename T, typename UL>
inline static T esp_scale8(UL i, UL scale) {
    return sat8<T, UL>(i * scale);
}

template<typename T, typename UL>
inline static uint8_t esp_div8(UL i, UL scale) {
    if (scale == 0) {
        return sat8<T, UL>(inf(i));
    }
    return sat8<T, UL>(i / scale);
}


} // namespace esphome
} // namespace waveshare_epaper
} // namespace elements
