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
#ifndef KIS_COMPOSITE_OP_H_
#define KIS_COMPOSITE_OP_H_

#include <map>
#include <qvaluelist.h>

//#include "kis_global.h"
#include "kis_id.h"

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

class KisCompositeOp {
public:
    KisCompositeOp();
    KisCompositeOp(const QString& id);
    KisCompositeOp(CompositeOp compositeOp);

    KisID id() const { return m_id; }
    CompositeOp op() const { return m_op; }

    bool isValid() const { return m_valid; }

    bool operator==(const KisCompositeOp& other) const;
    bool operator!=(const KisCompositeOp& other) const;

private:
    void fillMap();

private:
    CompositeOp m_op;
    KisID m_id;
    bool m_valid;

    typedef std::map<KisID, CompositeOp> KisIDCompositeOpMap;
    static KisIDCompositeOpMap s_idOpMap;
};

typedef QValueList<KisCompositeOp> KisCompositeOpList;

#endif // KIS_COMPOSITE_OP_H
