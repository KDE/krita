/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "RulerDecoration.h"

#include <math.h>

#include <QPainter>

#include <kdebug.h>
#include <klocale.h>

#include <KoViewConverter.h>

#include "Ruler.h"

RulerDecoration::RulerDecoration(KisView2* _view, Ruler* _ruler) : KisCanvasDecoration("ruler", i18n("Ruler decoration"), _view), m_ruler(_ruler),m_edition( false)
{
}

RulerDecoration::~RulerDecoration()
{

}

inline double angle( const QPointF& p1, const QPointF& p2)
{
    return atan2( p2.y() - p1.y(), p2.x() - p1.x());
}


inline double norm2(const QPointF& p)
{
    return sqrt(p.x() * p.x() + p.y() * p.y() );
}

void RulerDecoration::setEdition( bool v )
{
    m_edition = v;
}

void RulerDecoration::drawDecoration(QPainter& _painter, const QPoint & documentOffset, const QRect& _area, const KoViewConverter &_converter)
{
    Q_UNUSED(_area);
    // Draw the gradient
    _painter.save();
    {
        _painter.translate( _converter.documentToView( m_ruler->point1() ) );
        _painter.rotate( angle( m_ruler->point1(), m_ruler->point2() ) / M_PI * 180 );
        QLinearGradient gradient(0,-30,0,30);
        gradient.setColorAt(0, QColor(0,0,0,0));
        gradient.setColorAt(0.5, QColor(0,0,0,100));
        gradient.setColorAt(1, QColor(0,0,0,0));
        _painter.setBrush( gradient );
        _painter.setPen( QPen(Qt::NoPen) );
        _painter.drawRect( _converter.documentToView(QRectF(0, -50, norm2( m_ruler->point2() - m_ruler->point1() ), 100)));
    }
    _painter.restore();
    _painter.drawLine( _converter.documentToView(m_ruler->point1()),
                       _converter.documentToView(m_ruler->point2()));
    if(m_edition)
    { // Draw the handles
        _painter.setBrush( QColor(0,0,0,100) );
        _painter.drawEllipse( QRectF( -10, -10, 20, 20 ).translated( _converter.documentToView(m_ruler->point1()) ) );
        _painter.drawEllipse( QRectF( -10, -10, 20, 20 ).translated( _converter.documentToView(m_ruler->point2()) ) );
    }
}
