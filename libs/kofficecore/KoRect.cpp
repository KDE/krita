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

#include "KoRect.h"

KoRect KoRect::normalize() const
{
    KoRect r;
    if ( right() < left() ) {                            // swap bad x values
        r.m_tl.setX( right() );
        r.m_br.setX( left() );
    } else {
        r.m_tl.setX( left() );
        r.m_br.setX( right() );
    }
    if ( bottom() < top() ) {                            // swap bad y values
        r.m_tl.setY( bottom() );
        r.m_br.setY( top() );
    } else {
        r.m_tl.setY( top() );
        r.m_br.setY( bottom() );
    }
    return r;
}

void KoRect::setTopLeft(const KoPoint &topleft)
{
    m_tl = topleft;
}

void KoRect::setBottomRight(const KoPoint &bottomright)
{
    m_br = bottomright;
}

void KoRect::setTopRight(const KoPoint &topright)
{
    m_br.rx() = topright.x();
    m_tl.ry() = topright.y();
}

void KoRect::setBottomLeft(const KoPoint &bottomleft)
{
    m_tl.rx() = bottomleft.x();
    m_br.ry() = bottomleft.y();
}


KoPoint KoRect::center() const
{
  return KoPoint((left() + right()) / 2, (top() + bottom()) / 2);
}

void KoRect::moveTopLeft(const KoPoint &topleft)
{
    m_br.rx() += topleft.x() - m_tl.x();
    m_br.ry() += topleft.y() - m_tl.y();
    m_tl = topleft;
}

void KoRect::moveBottomRight(const KoPoint &bottomright)
{
    m_tl.rx() += bottomright.x() - m_br.x();
    m_tl.ry() += bottomright.y() - m_br.y();
    m_br = bottomright;
}

void KoRect::moveTopRight(const KoPoint &topright)
{
    m_tl.rx() += topright.x() - m_br.x();
    m_br.ry() += topright.y() - m_tl.y();
    m_br.rx() = topright.x();
    m_tl.ry() = topright.y();
}

void KoRect::moveBottomLeft(const KoPoint &bottomleft)
{
    m_br.rx() += bottomleft.x() - m_tl.x();
    m_tl.ry() += bottomleft.y() - m_br.y();
    m_tl.rx() = bottomleft.x();
    m_br.ry() = bottomleft.y();
}

void KoRect::moveBy(const double &dx, const double &dy)
{
    m_tl.rx() += dx;
    m_tl.ry() += dy;
    m_br.rx() += dx;
    m_br.ry() += dy;
}

void KoRect::setRect(const double &x, const double &y, const double &width, const double &height)
{
    m_tl.setCoords( x, y );
    m_br.setCoords( x + width, y + height );
}

void KoRect::setRect(const KoRect &rect)
{
    m_tl = rect.m_tl;
    m_br = rect.m_br;
}

void KoRect::setCoords(const double &x1, const double &y1, const double &x2, const double &y2)
{
    m_tl.setCoords( x1, y1 );
    m_br.setCoords( x2, y2 );
}

void KoRect::setSize(const KoSize &size)
{
    setWidth(size.width());
    setHeight(size.height());
}

KoSize KoRect::size() const
{
    return KoSize(width(), height());
}

KoRect &KoRect::operator|=(const KoRect &rhs) {

    if(rhs.isEmpty())
        return *this;
    if(isEmpty())
    {
        *this = rhs;
        return *this;
    }
    if(m_tl.x() > rhs.left())
        m_tl.setX(rhs.left());
    if(m_tl.y() > rhs.top())
        m_tl.setY(rhs.top());
    if(m_br.x() < rhs.right())
        m_br.setX(rhs.right());
    if(m_br.y() < rhs.bottom())
        m_br.setY(rhs.bottom());
    return *this;
}

KoRect &KoRect::operator&=(const KoRect &rhs) {

    if(m_tl.x() < rhs.left())
        m_tl.setX(rhs.left());
    if(m_tl.y() < rhs.top())
        m_tl.setY(rhs.top());
    if(m_br.x() > rhs.right())
        m_br.setX(rhs.right());
    if(m_br.y() > rhs.bottom())
        m_br.setY(rhs.bottom());
    return *this;
}

bool KoRect::contains(const KoPoint &p) const {
    return (p.x() >= m_tl.x() && p.x() <= m_br.x() && p.y() >= m_tl.y() && p.y() <= m_br.y());
}

bool KoRect::contains(const double &x, const double &y) const {
    return (x >= m_tl.x() && x <= m_br.x() && y >= m_tl.y() && y <= m_br.y());
}

bool KoRect::contains(const KoRect &r) const {
    return (r.left() >= m_tl.x() && r.right() <= m_br.x() && r.top() >= m_tl.y() && r.bottom() <= m_br.y());
}


KoRect KoRect::unite(const KoRect &r) const {
    return *this | r;
}

KoRect KoRect::intersect(const KoRect &r) const {
    return *this & r;
}

bool KoRect::intersects(const KoRect &r) const {
    return ( qMax(m_tl.x(), r.left()) <= qMin(m_br.x(), r.right()) &&
             qMax(m_tl.y(), r.top()) <= qMin(m_br.y(), r.bottom()) );
}

KoRect operator|(const KoRect &lhs, const KoRect &rhs) {

    if(lhs.isEmpty())
        return rhs;
    if(rhs.isEmpty())
        return lhs;
    KoRect tmp;
    tmp.setCoords( (lhs.left() < rhs.left() ? lhs.left() : rhs.left()),
                   (lhs.top() < rhs.top() ? lhs.top() : rhs.top()),
                   (lhs.right() > rhs.right() ? lhs.right() : rhs.right()),
                   (lhs.bottom() > rhs.bottom() ? lhs.bottom() : rhs.bottom()) );
    return tmp;
}

KoRect operator&(const KoRect &lhs, const KoRect &rhs) {

    KoRect tmp;
    tmp.setCoords( (lhs.left() > rhs.left() ? lhs.left() : rhs.left()),
                   (lhs.top() > rhs.top() ? lhs.top() : rhs.top()),
                   (lhs.right() < rhs.right() ? lhs.right() : rhs.right()),
                   (lhs.bottom() < rhs.bottom() ? lhs.bottom() : rhs.bottom()) );
    return tmp;
}

bool operator==(const KoRect &lhs, const KoRect &rhs) {
    return ( lhs.topLeft()==rhs.topLeft() &&
             lhs.bottomRight()==rhs.bottomRight() );
}

bool operator!=(const KoRect &lhs, const KoRect &rhs) {
    return ( lhs.topLeft()!=rhs.topLeft() ||
             lhs.bottomRight()!=rhs.bottomRight() );
}

KoRect KoRect::transform(const QMatrix &m) const
{
  KoRect result;
  if(m.m12() == 0.0F && m.m21() == 0.0F)
  {
    result = KoRect(topLeft().transform(m), bottomRight().transform(m));
  }
  else
  {
    int i;
    KoPoint p[4] = { KoPoint(m_tl.x(), m_tl.y()), KoPoint(m_tl.x(), m_br.x()),
                   KoPoint(m_br.x(), m_br.x()), KoPoint(m_br.x(), m_tl.y()) };
    for(i = 0; i < 4; i++)
      p[i] = p[i].transform(m);

    result.setLeft(p[0].x());
    result.setTop(p[0].y());
    result.setRight(p[0].x());
    result.setBottom(p[0].y());

    for(int i = 1; i < 4; i++)
    {
      result.setLeft(qMin(p[i].x(), result.left()));
      result.setTop(qMin(p[i].y(), result.top()));
      result.setRight(qMax(p[i].x(), result.right()));
      result.setBottom(qMax(p[i].y(), result.bottom()));
    }
  }
  return result;
}

KoRect KoRect::translate(double dx, double dy) const
{
  return KoRect(left() + dx, top() + dy, width(), height());
}

QRect KoRect::toQRect() const
{
    return QRect( qRound( left() ), qRound( top() ), qRound( width() ), qRound( height() ) );
}

//static
KoRect KoRect::fromQRect( const QRect &rect )
{
    return KoRect( rect.left(), rect.top(), rect.width(), rect.height() );
}
