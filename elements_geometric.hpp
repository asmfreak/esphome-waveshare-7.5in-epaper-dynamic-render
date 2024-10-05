#pragma once
#include <cstdint>
#include <cmath>
#include <array>
#include <algorithm>

namespace esphome {
namespace waveshare_epaper {
namespace elements {

enum class HasPoint: uint8_t{
    Outside=0,
    OnBorder,
    Inside
};

class Point2D {
public:
    int x;
    int y;
    bool operator<(const Point2D &other) const {
        if (x < other.x) return true;
        if (other.x < x) return false;
        return y < other.y;
    }
    inline Point2D operator+(const Point2D &add) const ESPHOME_ALWAYS_INLINE {
        Point2D ret;
        ret.x   = this->x   + add.x;
        ret.y = this->y + add.y;
        return ret;
    }
    inline Point2D &operator+=(const Point2D &add) ESPHOME_ALWAYS_INLINE { return *this = (*this) + add; }
    inline Point2D operator-(const Point2D &subtract) const ESPHOME_ALWAYS_INLINE {
        Point2D ret;
        ret.x   = this->x   - subtract.x;
        ret.y = this->y - subtract.y;
        return ret;
    }
    inline Point2D &operator-=(const Point2D &subtract) ESPHOME_ALWAYS_INLINE { return *this = (*this) - subtract; }
    
    float len() const {
        return ::sqrt(float(x)*float(x)+float(y)*float(y));
    }
    
    float cross(const Point2D& other) const {
        return x*other.y - y*other.x;
    }
};

class Rect2D {
public:
    Point2D tl;
    Point2D br;
    
    bool has(const Point2D& p) const {
        return \
               (p.x >= tl.x and p.y >= tl.y) and (p.x <= br.x and p.y <= br.y);
    }
    
    bool has(const Rect2D& p) const {
        return has(p.tl) && has(p.br);
    }
    
    int width() const{
        return br.x - tl.x;
    }
    int height() const{
        return br.y - tl.y;
    }
};

class Triangle2D {
    std::array<Point2D, 3> v;
    Rect2D bb;
    // int x3, y3;
    // int y23, x32, y31, x13;
    // int det, minD, maxD;
public:
    Triangle2D(Point2D a, Point2D b, Point2D c):v{a,b,c}{
        const auto mmx = std::minmax({v[0].x,v[1].x,v[2].x});
        const auto mmy = std::minmax({v[0].y,v[1].y,v[2].y});
        bb = Rect2D{
            Point2D{mmx.first, mmy.first},
            Point2D{mmx.second, mmy.second}
        };
        // x3 = c.x;
        // y3 = c.y;
        // y23 = b.y - c.y;
        // x32 = c.x - b.x;
        // y31 = y3 - a.y;
        // x13 = a.x - x3;
        // det = y23 * x13 - x32 * y31;
        // minD = std::min(det, 0);
        // maxD = std::max(det, 0);
    }
    bool has(const Point2D& pt) const
    {
        // taken from https://stackoverflow.com/a/2049593
        if(not bb.has(pt)){
            return false;
        }
        float d1, d2, d3;
        bool has_neg, has_pos;
        
        d1 = sign(pt, v[0], v[1]);
        d2 = sign(pt, v[1], v[2]);
        d3 = sign(pt, v[2], v[0]);
        
        has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        
        return !(has_neg && has_pos);
    }
    Rect2D boundingBox() const{
        return bb;
    }
private:
    static int sign(Point2D p1, Point2D p2, Point2D p3)
    {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }
};



class Circle2D {
public:
    Point2D center;
    float radius;
    
    HasPoint has(const Point2D& p) const {
        const auto plen = (center - p).len();
        if(::fabs(plen - radius) < 0.5){
            return HasPoint::OnBorder;
        }
        if(plen > radius){
            return HasPoint::Outside;
        }
        return HasPoint::Inside;
    }
    
    Rect2D boundingBox() const {
        auto rad = Point2D{int(radius), int(radius)};
        return Rect2D{
            center - rad,
            center + rad
        };
    }
};

} // namespace esphome
} // namespace waveshare_epaper
} // namespace elements
