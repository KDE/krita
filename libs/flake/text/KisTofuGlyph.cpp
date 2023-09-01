/*
 *  SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisTofuGlyph.h"

#include <QPainterPath>
#include <QPolygon>
#include <QTransform>
#include <QVector>

namespace KisTofuGlyph
{

// These functions build the QPainterPath for each hex char using polygons.
// Each char glyph is formed by 3x5 grid of squares constructed from polygons
// wound in the clockwise direction (counterclockwise to subtract).

static inline QPolygon upperHole()
{
    return {QVector<QPoint>{{1, 1}, {1, 2}, {2, 2}, {2, 1}}};
}

static inline QPolygon lowerHole()
{
    return {QVector<QPoint>{{1, 3}, {1, 4}, {2, 4}, {2, 3}}};
}

static inline QPainterPath hexChar0()
{
    static const QPainterPath s_path0 = []() {
        const QPolygon b{QVector<QPoint>{{1, 1}, {1, 4}, {2, 4}, {2, 1}}};
        QPainterPath p;
        p.addRect(0, 0, 3, 5);
        p.addPolygon(b);
        return p;
    }();
    return s_path0;
}

static inline QPainterPath hexChar1()
{
    static const QPainterPath s_path1 = []() {
        QPainterPath p;
        p.addRect(1, 0, 1, 5);
        return p;
    }();
    return s_path1;
}

static inline QPainterPath hexChar2()
{
    static const QPainterPath s_path2 = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {3, 0},
            {3, 3},
            {1, 3},
            {1, 4},
            {3, 4},
            {3, 5},
            {0, 5},
            {0, 2},
            {2, 2},
            {2, 1},
            {0, 1},
        }};
        QPainterPath p;
        p.addPolygon(a);
        return p;
    }();
    return s_path2;
}

static inline QPainterPath hexChar3()
{
    static const QPainterPath s_path3 = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {3, 0},
            {3, 5},
            {0, 5},
            {0, 4},
            {2, 4},
            {2, 3},
            {0, 3},
            {0, 2},
            {2, 2},
            {2, 1},
            {0, 1},
        }};
        QPainterPath p;
        p.addPolygon(a);
        return p;
    }();
    return s_path3;
}

static inline QPainterPath hexChar4()
{
    static const QPainterPath s_path4 = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {1, 0},
            {1, 2},
            {2, 2},
            {2, 0},
            {3, 0},
            {3, 5},
            {2, 5},
            {2, 3},
            {0, 3},
        }};
        QPainterPath p;
        p.addPolygon(a);
        return p;
    }();
    return s_path4;
}

static inline QPainterPath hexChar5()
{
    static const QPainterPath s_path5 = []() {
        // Just mirror a "2".
        QPainterPath p = hexChar2();
        return QTransform::fromScale(-1, 1).map(p).toReversed().translated(3, 0);
    }();
    return s_path5;
}

static inline QPainterPath hexChar6()
{
    static const QPainterPath s_path6 = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {3, 0},
            {3, 1},
            {1, 1},
            {1, 2},
            {3, 2},
            {3, 5},
            {0, 5},
        }};
        QPainterPath p;
        p.addPolygon(a);
        p.addPolygon(lowerHole());
        return p;
    }();
    return s_path6;
}

static inline QPainterPath hexChar7()
{
    static const QPainterPath s_path7 = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {3, 0},
            {3, 5},
            {2, 5},
            {2, 1},
            {0, 1},
        }};
        QPainterPath p;
        p.addPolygon(a);
        return p;
    }();
    return s_path7;
}

static inline QPainterPath hexChar8()
{
    static const QPainterPath s_path8 = []() {
        QPainterPath p;
        p.addRect(0, 0, 3, 5);
        p.addPolygon(upperHole());
        p.addPolygon(lowerHole());
        return p;
    }();
    return s_path8;
}

static inline QPainterPath hexChar9()
{
    static const QPainterPath s_path9 = []() {
        // Just rotate a "6" upside-down
        QPainterPath p = hexChar6();
        return QTransform::fromScale(-1, -1).map(p).translated(3, 5);
    }();
    return s_path9;
}

static inline QPainterPath hexCharA()
{
    static const QPainterPath s_pathA = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {3, 0},
            {3, 5},
            {2, 5},
            {2, 3},
            {1, 3},
            {1, 5},
            {0, 5},
        }};
        QPainterPath p;
        p.addPolygon(a);
        p.addPolygon(upperHole());
        return p;
    }();
    return s_pathA;
}

static inline QPainterPath hexCharB()
{
    static const QPainterPath s_pathB = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {2, 0},
            {2, 1},
            {3, 1},
            {3, 2},
            {2, 2},
            {2, 3},
            {3, 3},
            {3, 4},
            {2, 4},
            {2, 5},
            {0, 5},
        }};
        QPainterPath p;
        p.addPolygon(a);
        p.addPolygon(upperHole());
        p.addPolygon(lowerHole());
        return p;
    }();
    return s_pathB;
}

static inline QPainterPath hexCharC()
{
    static const QPainterPath s_pathC = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {3, 0},
            {3, 1},
            {1, 1},
            {1, 4},
            {3, 4},
            {3, 5},
            {0, 5},
        }};
        QPainterPath p;
        p.addPolygon(a);
        return p;
    }();
    return s_pathC;
}

static inline QPainterPath hexCharD()
{
    static const QPainterPath s_pathD = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {2, 0},
            {2, 1},
            {1, 1},
            {1, 4},
            {2, 4},
            {2, 5},
            {0, 5},
        }};
        const QPolygon b{QVector<QPoint>{{2, 1}, {3, 1}, {3, 4}, {2, 4}}};
        QPainterPath p;
        p.addPolygon(a);
        p.addPolygon(b);
        return p;
    }();
    return s_pathD;
}

static inline QPainterPath hexCharE()
{
    static const QPainterPath s_pathE = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {3, 0},
            {3, 1},
            {1, 1},
            {1, 2},
            {3, 2},
            {3, 3},
            {1, 3},
            {1, 4},
            {3, 4},
            {3, 5},
            {0, 5},
        }};
        QPainterPath p;
        p.addPolygon(a);
        return p;
    }();
    return s_pathE;
}

static inline QPainterPath hexCharF()
{
    static const QPainterPath s_pathF = []() {
        const QPolygon a{QVector<QPoint>{
            {0, 0},
            {3, 0},
            {3, 1},
            {1, 1},
            {1, 2},
            {3, 2},
            {3, 3},
            {1, 3},
            {1, 5},
            {0, 5},
        }};
        QPainterPath p;
        p.addPolygon(a);
        return p;
    }();
    return s_pathF;
}

static QPainterPath getHexChar(unsigned value)
{
    switch (value) {
    case 0x0:
        return hexChar0();
    case 0x1:
        return hexChar1();
    case 0x2:
        return hexChar2();
    case 0x3:
        return hexChar3();
    case 0x4:
        return hexChar4();
    case 0x5:
        return hexChar5();
    case 0x6:
        return hexChar6();
    case 0x7:
        return hexChar7();
    case 0x8:
        return hexChar8();
    case 0x9:
        return hexChar9();
    case 0xA:
        return hexCharA();
    case 0xB:
        return hexCharB();
    case 0xC:
        return hexCharC();
    case 0xD:
        return hexCharD();
    case 0xE:
        return hexCharE();
    case 0xF:
        return hexCharF();
    }
    return {};
}

/**
 * @brief Adds a hex char at the specified row/column to the QPainterPath.
 */
static void addHexChar(QPainterPath &p, unsigned value, int row, int col)
{
    QPainterPath glyph = getHexChar(value);
    glyph.translate(2 + col * 4, 2 + row * 6);
    p.addPath(glyph);
}

/**
 * @brief Gets the hex digit at a place.
 * 
 * @param codepoint
 * @param place 0-base digit index
 * @return the digit
 */
static constexpr unsigned valueAt(const char32_t codepoint, const unsigned place)
{
    return (codepoint >> (place * 4)) & 0xF;
}

/**
 * @brief Creates the frame of a tofu glyph
 */
static inline QPainterPath makeFrame(const int width)
{
    const int inner = width - 1;
    const QPolygon a{QVector<QPoint>{{0, 0}, {width, 0}, {width, 15}, {0, 15}}};
    const QPolygon b{QVector<QPoint>{{1, 1}, {1, 14}, {inner, 14}, {inner, 1}}};
    QPainterPath p;
    p.addPolygon(a);
    p.addPolygon(b);
    return p;
}

QPainterPath create(const char32_t codepoint, double height)
{
    // We build the glyph as a 15x15 or 11x15 grid of squares.
    QPainterPath p;
    if (codepoint > 0xFFFF) {
        // Codepoints outside the BMP need more than 4 digits to display, so we show 6.
        // +---+
        // |01F|
        // |389| => U+1F389
        // +---+
        static const QPainterPath s_outline15 = makeFrame(15);
        p.addPath(s_outline15);
        addHexChar(p, valueAt(codepoint, 5), 0, 0);
        addHexChar(p, valueAt(codepoint, 4), 0, 1);
        addHexChar(p, valueAt(codepoint, 3), 0, 2);
        addHexChar(p, valueAt(codepoint, 2), 1, 0);
        addHexChar(p, valueAt(codepoint, 1), 1, 1);
        addHexChar(p, valueAt(codepoint, 0), 1, 2);
    } else {
        // +--+
        // |27|
        // |64| => U+2764
        // +--+
        static const QPainterPath s_outline11 = makeFrame(11);
        p.addPath(s_outline11);
        addHexChar(p, valueAt(codepoint, 3), 0, 0);
        addHexChar(p, valueAt(codepoint, 2), 0, 1);
        addHexChar(p, valueAt(codepoint, 1), 1, 0);
        addHexChar(p, valueAt(codepoint, 0), 1, 1);
    }
    const auto scale = (1. / 15.) * height;
    return QTransform::fromScale(scale, scale).map(p);
}

} // namespace KisTofuGlyph
