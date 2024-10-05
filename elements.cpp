#include "elements.hpp"

#include <cmath>

namespace esphome {
namespace waveshare_epaper {
namespace elements {


constexpr static TripleColor BLACK=TripleColor{0x00};
constexpr static TripleColor WHITE=TripleColor{0x03};
constexpr static TripleColor YELLOW=TripleColor{0x04};

float colLen(Color3F c){
    return std::sqrt(c.red*c.red + c.green*c.green + c.blue*c.blue);
}

float getErr(Color3F c, Color3 avail){
    return colLen(c-Color3F(avail));
}

constexpr static Color3 black(0,0,0);
constexpr static Color3 white(255,255,255);
constexpr static Color3 yellow(220,180,0);

TripleColor col2bin(Color3 c){
    struct err2bin{
        float err;
        TripleColor c;

        bool operator <(const err2bin& c) const {
            return  err < c.err;
        }
    };


    err2bin berr{getErr(Color3F(c), black), BLACK};
    err2bin werr{getErr(Color3F(c), white), WHITE};
    err2bin yerr{getErr(Color3F(c), yellow), YELLOW};

    return
        std::min(
            yerr,
            std::min(berr, werr)
        )
        .c;
}

Color3 col2pallete(Color3F c){
    struct err2bin{
        float err;
        Color3 c;

        bool operator <(const err2bin& c) const {
            return  err < c.err;
        }
    };
    
    err2bin berr{getErr(c, black), black};
    err2bin werr{getErr(c, white), white};
    err2bin yerr{getErr(c, yellow), yellow};
    
    
    return Color3(
        std::min(
            yerr,
            std::min(berr, werr)
        )
        .c);
}

}  // namespace elements
}  // namespace waveshare_epaper
}  // namespace esphome
