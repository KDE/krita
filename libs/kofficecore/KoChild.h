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
#ifndef __koChild_h__
#define __koChild_h__

#include <qobject.h>
#include <qmatrix.h>
//Added by qt3to4:
#include <QPolygon>
#include <koffice_export.h>

/**
 * KoChild is an abstract base class that represents the geometry
 * associated with an embedded document. In general it handles its position
 * relative to the embedded document's parent.
 *
 * In detail it handles size, matrix operations and can give you
 * a clip region. It can deal with scaling, rotation etc. because it
 * makes heavy usage of QMatrix.
 *
 * After applying the matrix, viewGeometry() applies zooming, but can be
 * reimplemented to also apply e.g. some translation by the application
 * (e.g. for centering the page).
 *
 * @see KoDocumentChild KoViewChild
 */
class KOFFICECORE_EXPORT KoChild : public QObject
{
  Q_OBJECT
public:

  /**
   * The gadget generally identifies where a child has been hit (generally
   * by the mouse pointer).
   * Based on this information different actions can be taken, for example
   * moving the child or opening a context menu. NoGadget means that
   * this child has not been hit.
   *
   * @see #gadgetHitTest
   */
  enum Gadget { NoGadget, TopLeft, TopMid, TopRight, MidLeft, MidRight,
		BottomLeft, BottomMid, BottomRight, Move };

  KoChild( QObject *parent = 0, const char *name = 0 );
  virtual ~KoChild();

  /**
   *  Sets a new geometry for this child document.
   *  Use noEmit = true if you do not want the 'changed'-signal to be emitted
   */
  void setGeometry( const QRect &rect, bool noEmit = false );

  /**
   * @return the rectangle that would be used to display this
   *         child document if the child is not rotated or
   *         subject to some other geometric transformation.
   *         The rectangle is in the coordinate system of the parent,
   *         using unzoomed coordinates in points.
   *
   * @see #setGeometry
   */
  QRect geometry() const;

  /**
   * @return the region of this child part relative to the
   *         coordinate system of the parent.
   *         The region is transformed with the passed
   *         matrix.
   */
  virtual QRegion region( const QMatrix& = QMatrix() ) const;

  /**
   * @return the polygon which surrounds the child part. The points
   *         are in coordinates of the parent.
   *         The points are transformed with the
   *         passed matrix.
   */
  virtual QPolygon pointArray( const QMatrix &matrix = QMatrix() ) const;

  /**
   * Tests whether the part contains a certain point. The point is
   * in the coordinate system of the parent.
   */
  //virtual bool contains( const QPoint& ) const;

  /**
   * @return the effective bounding rect after all transformations.
   *         The coordinates of the rectangle are in the coordinate system
   *         of the parent.
   */
  QRect boundingRect() const;

  /**
   * Scales the content of the child part. However, that does not
   * affect the size of the child part.
   */
  virtual void setScaling( double x, double y );

  /**
   * @return the x axis scaling of the child part
   */
  virtual double xScaling() const;

  /**
   * @return the y axis scaling of the child part
   */
  virtual double yScaling() const;

  /**
   * Shears the content of the child part.
   */
  virtual void setShearing( double x, double y );

  /**
   * @return the x axis shearing of the child part
   */
  virtual double xShearing() const;

  /**
   * @return the y axis shearing of the child part
   */
  virtual double yShearing() const;

  /**
   * Sets the angle of rotation.
   */
  virtual void setRotation( double );

  /**
   * @return the angle of rotation
   */
  virtual double rotation() const;

  /**
   * Sets the center of the rotation to the point @p pos.
   */
  virtual void setRotationPoint( const QPoint& pos );

  /**
   * @return the center of the rotation
   */
  virtual QPoint rotationPoint() const;

  /**
   * @return true if the child part is an orthogonal rectangle relative
   *         to its parents coordinate system.
   */
  bool isRectangle() const;

  /**
   * Sets the clip region of the painter, so that only pixels of the
   * child part can be drawn.
   *
   * @param painter the painter do modify.
   * @param combine tells whether the new clip region is an intersection
   *        of the current region with the childs region or whether only
   *        the childs region is set.
   */
  virtual void setClipRegion( QPainter& painter, bool combine = true );

  /**
   * Transforms the painter (its worldmatrix and the clipping)
   * in such a way that the painter can be passed to the child part
   * for drawing.
   */
  virtual void transform( QPainter& painter );

  /**
   * Sets the position of the content relative to the child frame.
   * This can be used to create a border between the frame border
   * and the actual content.
   */
  virtual void setContentsPos( int x, int y );

  /**
   * @return the contents rectangle that is visible.
   *         This value depends on the scaling and the geometry.
   *         This is the value that is passed to KoDocument::paintContent.
   *
   * @see #xScaling #geometry
   */
  virtual QRect contentRect() const;

  /**
   * @return the region of the child frame.
   *         If solid is set to true the complete area of the child region
   *         is returned, otherwise only the child border is returned.
   */
  virtual QRegion frameRegion( const QMatrix& matrix = QMatrix(), bool solid = false ) const;

  /**
   * @return the frame geometry including a border (6 pixels) as a point
   *         array with 4 points, one for each corner, transformed by given matrix.
   */
  virtual QPolygon framePointArray( const QMatrix &matrix = QMatrix() ) const;

  /**
   * @return the current transformation of this child as matrix.
   *         This includes translation, scale, rotation, shearing.
   *
   * @see #updateMatrix
   */
  virtual QMatrix matrix() const;

  /**
   * Locks this child and stores the current transformation.
   * A locked child does not emit changed signals.
   *
   * This is useful if a series of changes are done on this
   * child and only the final result is of interest (GUI updating,...).
   *
   * @see #locked #unlock
   */
  void lock();

  /**
   * Unlocks this child and emits a changed signal.
   */
  void unlock();

  /**
   * If the child is locked, geometry changes
   * (including scaling, rotation, ...) are not backed up.
   *
   * As long as this child is locked, the backed up
   * geometry state can be recovered with oldPointArray.
   *
   * @return true when this child is locked.
   *
   * @see #locked #unlock #oldPointArray
   */
  bool locked() const;

  /**
   * @return the backed up geometry transformed by given matrix.
   */
  virtual QPolygon oldPointArray( const QMatrix &matrix );

  /**
   * Marks this child as either transparent or not.
   * @param transparent set this child to transparent (true)
   *           or opaque (false).
   *
   * @see #isTransparent
   */
  virtual void setTransparent( bool transparent );

  /**
   * It might be interesting for view updates and repainting in general
   * whether a child is transparent or not.
   * @return true when this child is marked as transparent.
   */
  virtual bool isTransparent() const;

  /**
   * Different actions are taken depending on where a child frame is
   * hit. Two gadgets are known: one for the border (5 pixels) and one
   * for the inner area.
   * @return the gadget identification for the hit area.
   * @param p the hit position.
   *
   * @see #Gadget
   */
  virtual Gadget gadgetHitTest( const QPoint& p );

signals:

  /**
   * Emitted every time this child changes, but only if this child is not
   * locked.
   * @see #locked
   */
  void changed( KoChild *thisChild );

protected:

  /**
   * @return point array with the 4 corners of given rectangle, which is
   *         transformed by given matrix.
   *
   *  @param matrix the transformation of r.
   *  @param r the rectangle for which the point array should be created.
   */
  virtual QPolygon pointArray( const QRect& r, const QMatrix& matrix = QMatrix() ) const;

  /**
   * Stores the current transformation of this child into a matrix.
   *
   * @see #matrix
   */
  virtual void updateMatrix();
private:

  class KoChildPrivate;
  KoChildPrivate *d;
};

#endif
