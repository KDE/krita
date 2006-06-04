/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KOCOMPOSITEOP_H
#define KOCOMPOSITEOP_H

#include <map>
#include <QList>
#include <koffice_export.h>
#include <KoID.h>

enum CompositeOp {
    COMPOSITE_OVER,
    COMPOSITE_IN,
    COMPOSITE_OUT,
    COMPOSITE_ATOP,
    COMPOSITE_XOR,
    COMPOSITE_PLUS,
    COMPOSITE_MINUS,
    COMPOSITE_ADD,
    COMPOSITE_SUBTRACT,
    COMPOSITE_DIFF,
    COMPOSITE_MULT,
    COMPOSITE_DIVIDE,
    COMPOSITE_DODGE,
    COMPOSITE_BURN,
    COMPOSITE_BUMPMAP,
    COMPOSITE_COPY,
    COMPOSITE_COPY_RED,
    COMPOSITE_COPY_GREEN,
    COMPOSITE_COPY_BLUE,
    COMPOSITE_COPY_OPACITY,
    COMPOSITE_CLEAR,
    COMPOSITE_DISSOLVE,
    COMPOSITE_DISPLACE,
#if 0
    COMPOSITE_MODULATE,
    COMPOSITE_THRESHOLD,
#endif
    COMPOSITE_NO,
    COMPOSITE_DARKEN,
    COMPOSITE_LIGHTEN,
    COMPOSITE_HUE,
    COMPOSITE_SATURATION,
    COMPOSITE_VALUE,
    COMPOSITE_COLOR,
    COMPOSITE_COLORIZE,
    COMPOSITE_LUMINIZE,
    COMPOSITE_SCREEN,
    COMPOSITE_OVERLAY,
    COMPOSITE_COPY_CYAN,
    COMPOSITE_COPY_MAGENTA,
    COMPOSITE_COPY_YELLOW,
    COMPOSITE_COPY_BLACK,
    COMPOSITE_ERASE,
    COMPOSITE_UNDEF
};

class PIGMENT_EXPORT KoCompositeOp {
public:
    KoCompositeOp();
    KoCompositeOp(const QString& id);
    KoCompositeOp(CompositeOp compositeOp);

    KoID id() const { return m_id; }
    CompositeOp op() const { return m_op; }

    bool isValid() const { return m_valid; }

    bool operator==(const KoCompositeOp& other) const;
    bool operator!=(const KoCompositeOp& other) const;

private:
    void fillMap();

private:
    CompositeOp m_op;
    KoID m_id;
    bool m_valid;

    typedef std::map<KoID, CompositeOp> KoIDCompositeOpMap;
    static KoIDCompositeOpMap s_idOpMap;
};

typedef QList<KoCompositeOp> KoCompositeOpList;

#endif // KOCOMPOSITEOP_H
