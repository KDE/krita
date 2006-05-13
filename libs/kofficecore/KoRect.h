/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef koRect_h
#define koRect_h

#include "KoPoint.h"
#include "KoSize.h"
#include <QRect>
#include "koffice_export.h"

/**
 * A rect whose coordinates are floating-point values ( "double"s ).
 * The API isn't documented, it's a perfect mirror of QRect.
 */
class KOFFICEUI_EXPORT KoRect {

public:
    KoRect()
        : m_tl(), m_br() {}
    KoRect(const KoPoint &topleft, const KoPoint &bottomright)
        : m_tl(topleft), m_br(bottomright) {}
    KoRect(const KoPoint &topleft, const KoSize &size)
        {m_tl = topleft; setSize(size);}
    KoRect(const double &left, const double &top, const double &width, const double &height)
        : m_tl(left,top), m_br(left+width,top+height) {}
    ~KoRect() {}

    bool isNull() const { return m_tl == m_br; }
    // Like QRect, a null KoRect is empty.
    bool isEmpty() const { return m_tl.x() > m_br.x() || m_tl.y() > m_br.y() || isNull(); }
    // Unlike QRect, a null KoRect is valid (0-sized).
    bool isValid() const { return m_tl.x() <= m_br.x() && m_tl.y() <= m_br.y(); }
    KoRect normalize() const;

    double left() const { return m_tl.x(); }
    double top() const { return m_tl.y(); }
    double right() const { return m_br.x(); }
    double bottom() const { return m_br.y(); }

    double& rLeft() { return m_tl.rx(); }
    double& rTop() { return m_tl.ry(); }
    double& rRight() { return m_br.rx(); }
    double& rBottom() { return m_br.ry(); }

    double x() const { return left(); }
    double y() const { return top(); }

    void setLeft(const double &left) { m_tl.setX(left); }
    void setTop(const double &top) { m_tl.setY(top); }
    void setRight(const double &right) { m_br.setX(right); }
    void setBottom(const double &bottom) { m_br.setY(bottom); }

    void setX(const double &x) { m_tl.setX(x); } //same as setLeft()
    void setY(const double &y) { m_tl.setY(y); } //same as setTop()

    KoPoint topLeft() const { return m_tl; }
    KoPoint bottomRight() const { return m_br; }
    KoPoint topRight() const { return KoPoint(m_br.x(), m_tl.y()); }
    KoPoint bottomLeft() const { return KoPoint(m_tl.x(), m_br.y()); }
    KoPoint center() const;

    void setTopLeft(const KoPoint &topleft);
    void setBottomRight(const KoPoint &bottomright);
    void setTopRight(const KoPoint &topright);
    void setBottomLeft(const KoPoint &bottomleft);

    void moveTopLeft(const KoPoint &topleft);
    void moveBottomRight(const KoPoint &bottomright);
    void moveTopRight(const KoPoint &topright);
    void moveBottomLeft(const KoPoint &bottomleft);
    //void moveCenter(const KoPoint &center);
    void moveBy(const double &dx, const double &dy);

    void setRect(const double &x, const double &y, const double &width, const double &height);
    void setRect(const KoRect &rect);
    void setCoords(const double &x1, const double &y1, const double &x2, const double &y2);

    KoSize size() const;
    double width() const { return m_br.x()-m_tl.x(); }
    double height() const { return m_br.y()-m_tl.y(); }
    void setWidth(const double &width) { m_br.setX(m_tl.x()+width); }
    void setHeight(const double &height) { m_br.setY(m_tl.y()+height); }
    void setSize(const KoSize &size);

    KoRect &operator|=(const KoRect &rhs);
    KoRect &operator&=(const KoRect &rhs);
    /**
     * Returns if the point is contained in the rect.
     * @param p the point to test
     * Will return true if the point is contained in the 2d area this rect represents, this means
     * that it will return true on everthing from the topleft() to the bottomright();
     * Note that for KoRect(0, 0, 100, 100)  the KoPoint(0, 0) as well as KoPoint(100, 100) are
     * mathmatically contained in the rect, this in contrary to pixel based rectangles.
     */
    bool contains(const KoPoint &p) const;
    /// Helper function for the above function.
    bool contains(const double &x, const double &y) const;
    /// Helper function for the above function.
    bool contains(const KoRect &r) const;
    KoRect unite(const KoRect &r) const;
    KoRect intersect(const KoRect &r) const;
    bool intersects(const KoRect &r) const;

    KoRect transform(const QMatrix &m) const;
    KoRect translate(double dx, double dy) const;

    QRect toQRect() const;
    static KoRect fromQRect( const QRect &rect );

private:
    KoPoint m_tl, m_br;
};

KOFFICEUI_EXPORT KoRect operator|(const KoRect &lhs, const KoRect &rhs);
KOFFICEUI_EXPORT KoRect operator&(const KoRect &lhs, const KoRect &rhs);
KOFFICEUI_EXPORT bool operator==(const KoRect &lhs, const KoRect &rhs);
KOFFICEUI_EXPORT bool operator!=(const KoRect &lhs, const KoRect &rhs);


/** Show the position and size of a rectangle (use within kDebug) */
#define DEBUGRECT(rc) (rc).x() << "," << (rc).y() << " " << (rc).width() << "x" << (rc).height()

//inline kdbgstream operator<<( kdbgstream str, const KoRect & r )  { str << "[" << r.left() << ", " << r.top() << " - " << r.right() << ", " << r.bottom() << "]"; return str; }
inline kdbgstream operator<<( kdbgstream str, const KoRect & r )  { str << "[" << r.left() << "," << r.top() << " " << r.width() << "x" << r.height() << "]"; return str; }
inline kndbgstream operator<<( kndbgstream str, const KoRect & )  { return str; }

/** Show the rectangles that form a region (use as a standalone statement) */
#define DEBUGREGION(reg) { Q3MemArray<QRect>rs=reg.rects(); for (int i=0;i<rs.size();++i) \
                           kDebug()<<"  "<<DEBUGRECT(rs[i] )<<endl; }
// You can now use kDebug() << theregion << endl; (kdebug.h)

#endif
