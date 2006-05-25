/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#ifndef KOGRAPHICBASE_H
#define KOGRAPHICBASE_H

#include <QMatrix>
#include <QVector>
#include <QBrush>

#include "KoViewConverter.h"

class QPainter;
class QRectF;
class QPainterPath;

class KoSelection;
class KoGfxEvent;
class KoShapeContainer;
class KoRepaintManager;
class KoShapeBorderModel;

/**
 * Base class for all flake objects. Flake objects extend this class
 * to allow themselves to be manipulated. This class just represents
 * a graphical shape in the document and can be manipulated by some default
 * tools in this library.
 * <p>Due to the limited responsibility of this class, the extending object
 * can have any data backend and is responsible for painting itself.
 * <p>We strongly suggest that any extending class will use a Model View
 * Controller (MVC) design where the View part is all in this class, as well
 * as the one that inharits from this one.  This allows the data that rests
 * in the model to be reused in different parts of the document. For example
 * by having two flake objects that show it. Or each showing a section of it.
 * <p>The KoShape data is completely in postscript-points (pt) (see KoUnit
 * for conversion methods to and from pt).
 */
class KoShape
{
public:
    /**
     * @brief Constructor
     */
    KoShape();

    /**
     * @brief Destructor
     */
    virtual ~KoShape();

    /**
     * @brief Paint the shape
     * The class extending this one is responsible for painting itself.  Since we do not
     * assume the shape is square the paint must also clear its background if it will draw
     * something transparant on top.
     * This can be done with a method like:
     * <code>
       painter.fillRect(converter.normalToView(QRectF(QPointF(0.0,0.0), size())), background());</code>
     * Or equavalent for non-square objects.
     * @param painter used for painting the shape
     * @param converter to convert between internal and view coordinates.
     * @see applyConversion()
     */
    virtual void paint(QPainter &painter, KoViewConverter &converter) = 0;

    /**
     * Paint non-print decorations specific for this type of shape.
     * The default implementation is empty.
     *
     * @param painter used for painting the shape
     * @param selected true if the shape is currently selected
     * @param converter to convert between internal and view coordinates.
     */
    virtual void paintDecorations(QPainter &painter, KoViewConverter &converter, bool selected);

    /**
     * @brief Scale the shape using the zero-point which is the top-left corner.
     * @see position()
     *
     * @param sx scale in x direction
     * @param sy scale in y direction
     */
    void scale( double sx, double sy );

    /**
     * Return the current scaling adjustment over the X axis.
     */
    double scaleX() const { return m_scaleX; }

    /**
     * Return the current scaling adjustment over the Y axis.
     */
    double scaleY() const { return m_scaleY; }

    /**
     * @brief Rotate the shape
     *
     * The shape will be rotated using the center of the shape using the size()
     *
     * @param angle set the rotation angle of the shape in degrees
     */
    void rotate( double angle );

    /**
     * Return the current rotation in degrees.
     */
    double rotation() const { return m_angle; }

    /**
     * @brief Shear the shape
     * The shape will be sheared using the zero-point which is the top-left corner.
     * @see position()
     *
     * @param sx shear in x direction
     * @param sy shear in y direction
     */
    void shear( double sx, double sy );

    /**
     * @brief Resize the shape
     *
     * @param size the new size of the shape.  This is different from scaling as
     * scaling is a so called secondairy operation which is comparable to zooming in
     * instead of changing the size of the basic shape.
     * Easiest example of this difference is that using this method will not distort the
     * size of pattern-fills and borders.
     */
    virtual void resize( const QSizeF &size );

    /**
     * @brief Get the size of the shape in pt.
     *
     * @return the size of the shape as set by resize()
     */
    virtual QSizeF size() const { return m_size; }

    /**
     * @brief Set the position of the shape in pt
     *
     * @param position the new position of the shape
     */
    virtual void setPosition( const QPointF &position );

    /**
     * @brief Get the position of the shape in pt
     *
     * @return the position of the shape
     */
    virtual QPointF position() const { return m_pos; }

    /**
     * @brief Check if the shape is hit on position
     * @param position the position where the user clicked.
     * @return true when it hits.
     */
    virtual bool hitTest( const QPointF &position ) const;

    /**
     * @brief Get the bounding box of the shape
     *
     * This includes the line width but not the shadow of the shape
     *
     * @return the bounding box of the shape
     */
    virtual QRectF boundingBox() const;

    /**
     * @brief Add a connector point to the shape
     * A connector is a place on the shape that allows a graphical connection to be made
     * using a line, for example.
     *
     * @param point the position where to place the connector. The points coordinate system
     *   are based around the zero-pos which is the top-left of the shape
     *   The point does not have to be inside the boundings rectangle.  The point is in pt,
     *   just like the rest of the KoShape class uses.
     */
    void addConnectionPoint( const QPointF &point ) { m_connectors.append( point ); }

    /**
     * Set the background of the shape.
     * A QBrush can have a plain color, be fully transparant or have a complex fill.
     * setting such a brush will allow the shape to fill itself using that brush and
     * will be able to tell if its transparant or not.
     * @param brush the brush for the background.
     */
    void setBackground ( const QBrush & brush ) { m_backgroundBrush = brush; }

    /**
     * return the brush used to paint te background of this shape with.
     * A QBrush can have a plain color, be fully transparant or have a complex fill.
     * setting such a brush will allow the shape to fill itself using that brush and
     * will be able to tell if its transparant or not.
     * @return the background-brush
     */
    const QBrush& background () { return m_backgroundBrush; }

    /**
     * Returns true if there is some transparancy, false if the shape is fully opaque.
     * The default implementation will just return if the background has some transparancy,
     * you should override it and always return true if your shape is not square.
     * @return if the shape is (partly) transparant.
     */
    virtual bool hasTransparancy() { return !m_backgroundBrush.isOpaque(); }

    /**
     * Retrieve the z-coordinate of this shape.
     * The zIndex property is used to determine which shape lies on top of other objects.
     * An shape with a higher z-order is on top, and can obscure another shape.
     * @return the z-index of this shape.
     * @see setZIndex()
     */
    int zIndex() const;

    /**
     * Set the z-coordinate of this shape.
     * The zIndex property is used to determine which shape lies on top of other objects.
     * An shape with a higher z-order is on top, and can obscure, another shape.
     * <p>Just like two objects having the same x or y coordinate will make them 'touch',
     * so will two objects with the same z-index touch on the z plane.  In layering the
     * shape this, however, can cause a little confusion as one always has to be on top.
     * The layering if two overlapping objects have the same index is implementation dependant
     * and probably depends on the order in which they are added to the shape manager.
     * @param zIndex the new z-index;
     */
    void setZIndex(int zIndex) { m_zIndex = zIndex; }

    /**
     * Changes the Shape to be visible or invisible.
     * Being visible means being painted and printed, as well as being used for
     *   things like guidelines or searches.
     * @param on when true; set the shape to be visible.
     */
    void setVisible(bool on) { m_visible = on; }
    /**
     * Returns current visibility state of this shape.
     * Being visible means being painted and printed, as well as being used for
     *   things like guidelines or searches.
     * @return current visibility state of this shape.
     */
    bool isVisible() const { return m_visible; }

    /**
     * Changes the Shape to be locked in place.
     * Being locked means the shape can no longer change shape or position.
     * @param locked when true; set the shape to be locked.
     */
    void setLocked(bool locked) { m_locked = locked; }
    /**
     * Returns current locked state of this shape.
     * Being locked means the shape can no longer change shape or position.
     * @return current locked state of this shape.
     */
    bool isLocked() const { return m_locked; }

    /**
     * Returns the parent, or 0 if there is no parent.
     * @return the parent, or 0 if there is no parent.
     */
    KoShapeContainer *parent() const { return m_parent; }

    /**
     * Set the parent of this shape.
     * @param parent the new parent of this shape. Can be 0 if the shape has no parent anymore.
     */
    void setParent(KoShapeContainer *parent);

    /**
     * Request a repaint to be queued.
     * The repaint will be of the entire Shape, including its selection handles should this
     * shape be selected.
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     */
    virtual void repaint() const;

    /**
     * Request a repaint to be queued.
     * The repaint will be restricted to the parameters rectangle, which is expected to be
     * in points (the internal coordinates system of KoShape) and it is expected to be
     * normalized.
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     * @param shape the rectangle (in pt) to queue for repaint.
     */
    void repaint(QRectF &shape) const;

    /**
     * Request a repaint to be queued.
     * The repaint will be restricted to the parameters rectangle, which is expected to be
     * in points (the internal coordinates system of KoShape).
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     * @param x the x coordinate measured from the topleft position of this shape
     * @param y the y coordinate measured from the topleft position of this shape
     * @param width the width of the repaint rectangle
     * @param height the height of the repaint rectangle
     */
    void repaint(double x, double y, double width, double height) const;

    /**
     * This is a method used to sort a list using the STL sorting methods.
     * @param g1 the first shape
     * @param g2 the second shape
     */
    static bool compareShapeZIndex(KoShape *g1, KoShape *g2);

    /**
     * Called internally whenever a property is changed that requires the matrix to be recalculated.
     */
    virtual void recalcMatrix();

    /**
     * returns the outline of the shape in the form of a path.
     * The outline returned will always have the position() of the shape as the origin, so
     * moving the shape will not alter the result.  The outline is used to draw the border
     * on, for example.
     * @returns the outline of the shape in the form of a path.
     */
    virtual const QPainterPath outline() const;

    /**
     * Returns the currently set border, or 0 if there is no border.
     * @return the currently set border, or 0 if there is no border.
     */
    KoShapeBorderModel *border() { return m_border; }

    /**
     * Set a new border, removing the old one.
     * @param border the new border, or 0 if there should be no border.
     */
    void setBorder(KoShapeBorderModel *border) { m_border = border; }

    /**
     * Setting the shape to keep its aspect-ratio has the effect that user-scaling will
     * keep the width/hight ratio intact so as not to distort shapes that rely on that
     * ratio.
     * @param keepAspect the new value
     */
    void setKeepAspectRatio(bool keepAspect) { m_keepAspect = keepAspect; }
    /**
     * Setting the shape to keep its aspect-ratio has the effect that user-scaling will
     * keep the width/hight ratio intact so as not to distort shapes that rely on that
     * ratio.
     * @return whether to keep aspect ratio of this shape
     */
    bool keepAspectRatio() { return m_keepAspect; }

    /**
     * Return the position of this shape regardless of rotation/skew/scaling and regardless of
     * this shape having a parent (being in a group) or not.<br>
     * The returned value is the center of the shape.
     * @return the point that is the absolute, centered position of this shape.
     */
    QPointF absolutePosition() const;
    /**
     * Move this shape to an absolute position where the end location will be the same
     * regardless of the shape's rotation/skew/scaling and regardless of this shape having
     * a parent (being in a group) or not.<br>
     * The newPosition is going to be the center of the shape.
     * This has the convenient effect that: <pre>
    shape-&gt;setAbsolutePosition(QPointF(0,0));
    shape-&gt;rotate(45);</pre>
        Will result in the same visual position of the shape as the opposite:<pre>
    shape-&gt;rotate(45);
    shape-&gt;setAbsolutePosition(QPointF(0,0));</pre>
     * @param newPosition the new absolute center of the shape.
     */
    void setAbsolutePosition(QPointF newPosition);

protected:
    QMatrix m_invMatrix;
    QBrush m_backgroundBrush; ///< Stands for the background color / fill etc.
    KoShapeBorderModel *m_border; ///< points to a border, or 0 if there is no border

    /**
     * Convenience method that allows people implementing paint() to use the shape
     * internal coordinate system directly to paint itself instead of considering the
     * views zoom.
     * @param painter the painter to alter the zoom level of.
     * @param converter the converter for the current views zoom.
     */
    static void applyConversion(QPainter &painter, const KoViewConverter &converter);

private:
    double m_scaleX;
    double m_scaleY;
    double m_angle; // degrees
    double m_shearX;
    double m_shearY;

    QSizeF m_size; // size in pt
    QPointF m_pos; // position (top left) in pt

    QMatrix m_matrix;

    QVector<QPointF> m_connectors; // in pt

    int m_zIndex;
    KoShapeContainer *m_parent;

    bool m_visible, m_locked, m_keepAspect;

    KoRepaintManager *m_repaintManager;

private:
    friend class KoShapeManager;
    friend class KoShapeContainer;
    void setRepaintManager(KoRepaintManager *manager);

    /**
     * Create a matrix that describes all the transformations done on this shape.
     * @param converter if not null, this method uses the converter to mark the right
     *        offsets in the current view.
     */
    QMatrix KoShape::transformationMatrix(KoViewConverter *converter) const;

};

#endif /* KOGRAPHICBASE_H */
