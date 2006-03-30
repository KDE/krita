/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <KoChild.h>

#include <qpainter.h>
//Added by qt3to4:
#include <QPolygon>

#include <kdebug.h>

class KoChild::KoChildPrivate
{
public:
  KoChildPrivate()
  {
      m_contentsX = m_contentsY = 0;
  }
  ~KoChildPrivate()
  {
  }

  QRect m_geometry;

  double m_rotation;
  double m_shearX;
  double m_shearY;
  QPoint m_rotationPoint;
  double m_scaleX;
  double m_scaleY;
  QMatrix m_matrix;
  bool m_lock;
  QPolygon m_old;
  bool m_transparent;
  int m_contentsX;
  int m_contentsY;
};

KoChild::KoChild( QObject *parent, const char *name )
: QObject( parent, name )
{
  d = new KoChildPrivate;

  d->m_scaleX = d->m_scaleY = 1.0;
  d->m_shearX = d->m_shearY = 0.0;
  d->m_rotation = 0.0;
  d->m_lock = false;
  d->m_transparent = false;

  updateMatrix();
}

KoChild::~KoChild()
{
  delete d;
}

void KoChild::setGeometry( const QRect &rect, bool noEmit )
{
  if ( !d->m_lock )
    d->m_old = framePointArray();

  d->m_geometry = rect;

  // Embedded objects should have a minimum size of 3, so they can be selected
  if( d->m_geometry.width() < 3 )
    d->m_geometry.setWidth( 3 );

  if( d->m_geometry.height() < 3 )
    d->m_geometry.setHeight( 3 );

  updateMatrix();

  if ( !d->m_lock && !noEmit )
    emit changed( this );
}

QRect KoChild::geometry() const
{
  return d->m_geometry;
}

QRegion KoChild::region( const QMatrix &matrix ) const
{
  return QRegion( pointArray( matrix ) );
}

QPolygon KoChild::pointArray( const QMatrix &matrix ) const
{
  return pointArray( QRect( 0, 0, d->m_geometry.width(), d->m_geometry.height() ), matrix );
}

//bool KoChild::contains( const QPoint &point ) const
//{
//  return region().contains( point );
//}

QRect KoChild::boundingRect() const
{
  return pointArray().boundingRect();
}

bool KoChild::isRectangle() const
{
  return !( d->m_shearX != 0.0 || d->m_shearY != 0.0 || d->m_rotation != 0.0 );
}

void KoChild::setClipRegion( QPainter &painter, bool combine )
{
  painter.setClipping( true );
  if ( combine && !painter.clipRegion().isEmpty() )
    painter.setClipRegion( region( painter.worldMatrix() ).intersect( painter.clipRegion() ) );
  else
    painter.setClipRegion( region( painter.worldMatrix() ) );
}

void KoChild::setScaling( double x, double y )
{
  if ( !d->m_lock )
    d->m_old = framePointArray();

  d->m_scaleX = x;
  d->m_scaleY = y;

  // why is that commented out? (Simon)
  // This is commented out, because KoChild::transform() scales
  // the world matrix explicitly and updateMatrix() doesn't even
  // handle scaling (Werner)
  //updateMatrix()

  if ( !d->m_lock )
    emit changed( this );
}

double KoChild::xScaling() const
{
  return d->m_scaleX;
}

double KoChild::yScaling() const
{
  return d->m_scaleY;
}

void KoChild::setShearing( double x, double y )
{
  if ( !d->m_lock )
    d->m_old = framePointArray();

  d->m_shearX = x;
  d->m_shearY = y;

  updateMatrix();

  if ( !d->m_lock )
    emit changed( this );
}

double KoChild::xShearing() const
{
  return d->m_shearX;
}

double KoChild::yShearing() const
{
  return d->m_shearY;
}

void KoChild::setRotation( double rot )
{
  if ( !d->m_lock )
    d->m_old = framePointArray();

  d->m_rotation = rot;
  updateMatrix();

  if ( !d->m_lock )
    emit changed( this );
}

double KoChild::rotation() const
{
  return d->m_rotation;
}

void KoChild::setRotationPoint( const QPoint &pos )
{
  if ( !d->m_lock )
    d->m_old = framePointArray();

  d->m_rotationPoint = pos;
  updateMatrix();

  if ( !d->m_lock )
    emit changed( this );
}

QPoint KoChild::rotationPoint() const
{
  return d->m_rotationPoint;
}

void KoChild::transform( QPainter &painter )
{
    setClipRegion( painter, true );

    QMatrix m = painter.worldMatrix();
    m = d->m_matrix * m;
    m.scale( d->m_scaleX, d->m_scaleY );
    painter.setMatrix( m );
}

void KoChild::setContentsPos( int x, int y )
{
    d->m_contentsX = x;
    d->m_contentsY = y;
}

QRect KoChild::contentRect() const
{
  return QRect( d->m_contentsX, d->m_contentsY, int(d->m_geometry.width() / d->m_scaleX),
                int(d->m_geometry.height() / d->m_scaleY) );
}

QPolygon KoChild::framePointArray( const QMatrix &matrix ) const
{
  return pointArray( QRect( -6, -6, d->m_geometry.width() + 12, d->m_geometry.height() + 12 ), matrix );
}

QRegion KoChild::frameRegion( const QMatrix &matrix, bool solid ) const
{
  const QPolygon arr = framePointArray( matrix );
  const QRegion frameReg( arr );

  if ( solid )
    return frameReg;

  const QRegion reg = region( matrix );
  return frameReg.subtract( reg );
}

QPolygon KoChild::pointArray( const QRect &r, const QMatrix &matrix ) const
{
  QPoint topleft = d->m_matrix.map( QPoint( r.left(), r.top() ) );
  QPoint topright = d->m_matrix.map( QPoint( r.right(), r.top() ) );
  QPoint bottomleft = d->m_matrix.map( QPoint( r.left(), r.bottom() ) );
  QPoint bottomright = d->m_matrix.map( QPoint( r.right(), r.bottom() ) );

  QPolygon arr( 4 );
  arr.setPoint( 0, topleft );
  arr.setPoint( 1, topright );
  arr.setPoint( 2, bottomright );
  arr.setPoint( 3, bottomleft );

  for( int i = 0; i < 4; ++i )
      arr.setPoint( i, matrix.map( arr.point( i ) ) );

  return arr;
}

void KoChild::updateMatrix()
{
  QMatrix r;
  r.rotate( - d->m_rotation );
  QPoint p = r.map( QPoint( d->m_rotationPoint.x(),
			    d->m_rotationPoint.y() ) );

  QMatrix m;
  m.rotate( d->m_rotation );
  m.translate( -d->m_rotationPoint.x() + d->m_geometry.x(), -d->m_rotationPoint.y() + d->m_geometry.y() );
  m.translate( p.x(), p.y() );
  m.shear( d->m_shearX, d->m_shearY );

  d->m_matrix = m;
}

QMatrix KoChild::matrix() const
{
  return d->m_matrix;
}

void KoChild::lock()
{
  if ( d->m_lock )
    return;

  d->m_old = framePointArray();
  d->m_lock = true;
}

void KoChild::unlock()
{
  if ( !d->m_lock )
    return;

  d->m_lock = false;
  emit changed( this );
}

bool KoChild::locked() const
{
  return d->m_lock;
}

QPolygon KoChild::oldPointArray( const QMatrix &matrix )
{
  QPolygon arr = d->m_old;

  for( int i = 0; i < 4; ++i )
      arr.setPoint( i, matrix.map( arr.point( i ) ) );

  return arr;
}

void KoChild::setTransparent( bool transparent )
{
  d->m_transparent = transparent;
}

bool KoChild::isTransparent() const
{
  return d->m_transparent;
}

KoChild::Gadget KoChild::gadgetHitTest( const QPoint &p )
{
  if ( !frameRegion().contains( p ) )
    return NoGadget;

  if ( QRegion( pointArray( QRect( -5, -5, 5, 5 ) ) ).contains( p ) )
      return TopLeft;
  if ( QRegion( pointArray( QRect( d->m_geometry.width() / 2 - 3, -5, 5, 5 ) ) ).contains( p ) )
      return TopMid;
  if ( QRegion( pointArray( QRect( d->m_geometry.width(), -5, 5, 5 ) ) ).contains( p ) )
      return TopRight;
  if ( QRegion( pointArray( QRect( -5, d->m_geometry.height() / 2 - 3, 5, 5 ) ) ).contains( p ) )
      return MidLeft;
  if ( QRegion( pointArray( QRect( -5, d->m_geometry.height(), 5, 5 ) ) ).contains( p ) )
      return BottomLeft;
  if ( QRegion( pointArray( QRect( d->m_geometry.width() / 2 - 3,
				   d->m_geometry.height(), 5, 5 ) ) ).contains( p ) )
    return BottomMid;
  if ( QRegion( pointArray( QRect( d->m_geometry.width(), d->m_geometry.height(), 5, 5 ) ) ).contains( p ) )
      return BottomRight;
  if ( QRegion( pointArray( QRect( d->m_geometry.width(),
				   d->m_geometry.height() / 2 - 3, 5, 5 ) ) ).contains( p ) )
    return MidRight;

  return Move;
}

#include <KoChild.moc>
