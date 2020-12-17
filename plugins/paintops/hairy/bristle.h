/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _BRISTLE_H_
#define _BRISTLE_H_

#include <cmath>
#include <KoColor.h>

class Bristle
{

public:
    Bristle() = default;
    Bristle(float x, float y, float length);
    ~Bristle();

    inline float x() const {
        return m_x;
    }

    inline float y() const {
        return m_y;
    }

    inline float prevX() const {
        return m_prevX;
    }

    inline float prevY() const {
        return m_prevY;
    }

    inline float length() const {
        return m_length;
    }

    inline  const KoColor &color() const {
        return m_color;
    }

    inline int counter() const {
        return m_counter;
    }

    inline void upIncrement() {
        m_counter++;
    }

    inline float inkAmount() const {
        return m_inkAmount;
    };

    inline float distanceCenter() {
        return std::sqrt(m_x * m_x + m_y * m_y);
    }

    inline void setX(float x) {
        m_x = x;
    }
    inline void setY(float y) {
        m_y = y;
    }

    inline void setPrevX(float prevX) {
        m_prevX = prevX;
    }

    inline void setPrevY(float prevY) {
        m_prevY = prevY;
    }

    inline bool enabled() const {
        return m_enabled;
    }

    void setLength(float length);
    void setColor(const KoColor &color);

    void addInk(float value);
    void removeInk(float value);
    void setInkAmount(float inkAmount);
    void setEnabled(bool enabled);


private:
    void init(float x, float y, float length);

    // coordinates of bristle
    float m_x{0.0f};
    float m_y{0.0f};
    float m_prevX{0.0f};
    float m_prevY{0.0f};
    float m_length{0.0f}; // z - coordinate
    KoColor m_color;
    float m_inkAmount{0.0f};

    // new dimension in bristle
    int m_counter{0};

    bool m_enabled{true};
};

#endif
