/*
    SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef H_KIS_RADIAN_H
#define H_KIS_RADIAN_H

#include <cmath>

#define PI       3.14159265358979323846
#define PI2      6.28318530717958647693 // PI * 2.0
#define TO_DEG   57.2957795130823208768 // 180.0 / PI
#define TO_RAD   0.01745329251994329577 // PI / 180.0
#define RAD_360  6.28318530717958647693 // 360  = 2.0*PI
#define RAD_270  4.71238898038468985769 // 270° = 3.0*PI / 2.0
#define RAD_180  3.14159265358979323846 // 180° = PI
#define RAD_90   1.57079632679489661923 // 90°  = PI/2.0


template<class TReal> class KisRadian;

namespace _Private
{
struct Value
{
    template<class T>
    inline static const T& get(const T& value) { return value; }

    template<class T>
    inline static const T& get(const KisRadian<T>& rad) { return rad.value(); }
};
}

template<class TReal>
class KisRadian
{
public:

    KisRadian():
        m_value(TReal(0)) { }

    template<class U>
    KisRadian(const KisRadian<U>& rad):
        m_value(rad.m_value) { }
    
    template<class U>
    KisRadian(const U& rad) {
        m_value = normalizeRadians(_Private::Value::get(rad));
    }
    
    static TReal normalizeRadians(TReal rad) {
        rad = std::fmod((TReal)rad, (TReal)PI2);
        return rad < TReal(0) ? (rad + PI2) : rad;
    }
    
    static TReal normalizeDegrees(TReal deg) {
        deg = std::fmod(deg, TReal(360));
        return deg < TReal(0) ? (deg + TReal(360)) : deg;
    }
    
    static KisRadian from90Deg() {
        KisRadian rad;
        rad.m_value = RAD_90;
        return rad;
    }
    
    static KisRadian from180Deg() {
        KisRadian rad;
        rad.m_value = RAD_180;
        return rad;
    }
    
    static KisRadian from270Deg() {
        KisRadian rad;
        rad.m_value = RAD_270;
        return rad;
    }
    
    static KisRadian fromDegrees(const TReal& deg) { return KisRadian(deg * TO_RAD); }
    static TReal     toRadians  (const TReal& deg) { return normalizeDegrees(deg) * TO_RAD; }
    static TReal     toDegrees  (const TReal& rad) { return normalizeRadians(rad) * TO_DEG; }
    
    const TReal& value  () const { return m_value;          }
    TReal        degrees() const { return m_value * TO_DEG; }
    
    TReal scaled(const TReal& min, const TReal& max) const {
        return min + (m_value / PI2) * (max - min);
    }
    
    // ------ operator = ---------------------------------------------------- //
    
    template<class U>
    KisRadian& operator = (const U& rad) {
        m_value = normalizeRadians(_Private::Value::get(rad));
        return *this;
    }
    
    template<class U>
    KisRadian& operator = (const KisRadian<U>& rad) {
        m_value = rad.m_value;
        return *this;
    }
    
    // ------ operator + ---------------------------------------------------- //
    
    template<class U>
    KisRadian& operator += (const U& rad) {
        m_value = normalizeRadians(m_value + _Private::Value::get(rad));
        return *this;
    }
    
    
    friend KisRadian operator + (const KisRadian& l, const KisRadian& r) {
        KisRadian rad(l);
        rad += r;
        return rad;
    }
    
    // ------ operator - ---------------------------------------------------- //
    
    template<class U>
    KisRadian& operator -= (const U& rad) {
        m_value = normalizeRadians(m_value - _Private::Value::get(rad));
        return *this;
    }
    
    friend KisRadian operator - (const KisRadian& l, const KisRadian& r) {
        KisRadian rad(l);
        rad -= r;
        return rad;
    }
    
    // ------ operator * ---------------------------------------------------- //
    
    template<class U>
    KisRadian& operator *= (const U& rad) {
        m_value = normalizeRadians(m_value * _Private::Value::get(rad));
        return *this;
    }

    friend KisRadian operator * (const KisRadian& l, const KisRadian& r) {
        KisRadian rad(l);
        rad *= r;
        return rad;
    }
    
    // ------ operator / ---------------------------------------------------- //
    
    template<class U>
    KisRadian& operator /= (const U& rad) {
        m_value = normalizeRadians(m_value / _Private::Value::get(rad));
        return *this;
    }

    friend KisRadian operator / (const KisRadian& l, const KisRadian& r) {
        KisRadian rad(l);
        rad /= r;
        return rad;
    }
    
    // ------ operator % ---------------------------------------------------- //
    
    template<class U>
    KisRadian& operator %= (const U& rad) {
        m_value = normalizeRadians(std::fmod(m_value, TReal(_Private::Value::get(rad))));
        return *this;
    }
    
    
    friend KisRadian operator % (const KisRadian& l, const KisRadian& r) {
        KisRadian rad(l);
        rad %= r;
        return rad;
    }
    
private:
    TReal m_value;
};

#endif // H_KIS_RADIAN_H
