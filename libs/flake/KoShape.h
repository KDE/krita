/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOSHAPE_H
#define KOSHAPE_H

#include "KoInsets.h"

#include <QMatrix>
#include <QVector>
#include <QSet>
#include <QBrush>

#include <flake_export.h>

class QPainter;
class QRectF;
class QPainterPath;
class QVariant;

class KoSelection;
class KoPointerEvent;
class KoShapeContainer;
class KoShapeBorderModel;
class KoShapeManager;
class KoShapeUserData;
class KoViewConverter;
class KoShapeApplicationData;
class KoShapePrivate;
class KoShapeSavingContext;
class KoCanvasBase;
class KoGenStyle;

/**
 *
 * Base class for all flake shapes. Shapes extend this class
 * to allow themselves to be manipulated. This class just represents
 * a graphical shape in the document and can be manipulated by some default
 * tools in this library.
 *
 * Due to the limited responsibility of this class, the extending object
 * can have any data backend and is responsible for painting itself.
 *
 * We strongly suggest that any extending class will use a Model View
 * Controller (MVC) design where the View part is all in this class, as well
 * as the one that inherits from this one.  This allows the data that rests
 * in the model to be reused in different parts of the document. For example
 * by having two flake objects that show that same data. Or each showing a section of it.
 *
 * The KoShape data is completely in postscript-points (pt) (see KoUnit
 * for conversion methods to and from points).
 * This image will explain the real-world use of the shape and its options.
 * <img src="../flake_shape_coords.png" align=center><br>
 *  The Rotation center can be returned with absolutePosition()
 *
 * <p>Flake objects can be created in three ways:
 * <ul>
 *   <li>a simple new KoDerivedFlake(),
 *   <li>through an associated tool,
 *   <li>through a factory
 * </ul>
 */
class FLAKE_EXPORT KoShape
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
     * something transparent on top.
     * This can be done with a method like:
     * <code>
       painter.fillRect(converter.normalToView(QRectF(QPointF(0.0,0.0), size())), background());</code>
     * Or equavalent for non-square objects.
     * Do note that a shape's top-left is always at coordinate 0,0. Even if the shape itself is rotated
     * or translated.
     * @param painter used for painting the shape
     * @param converter to convert between internal and view coordinates.
     * @see applyConversion()
     */
    virtual void paint(QPainter &painter, const KoViewConverter &converter) = 0;

    /**
     * Paint non-print decorations specific for this type of shape.
     * The default implementation is empty.
     *
     * @param painter used for painting the shape
     * @param converter to convert between internal and view coordinates.
     * @param canvas the canvas that requested this paint.  This can be used to retrieve canvas specific properties
     *      like selection and get a reference to the KoCanvasResourceProvider.
     */
    virtual void paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas);

    /**
     * @brief Save the shape
     *
     * This is the method that will be called when saving a shape as a described in 
     * OpenDocument 9.2 Drawing Shapes. This method saves the common attributes of the
     * drawing shapes.
     * The special data for every shape is saved in @see saveOdfData().
     *
     * Please don't use yet as the design is not yet finished.
     *
     * @see saveOdfContent
     */
    bool saveOdf( KoShapeSavingContext & context );

    /**
     * @brief Save the shape
     *
     * This is used to save a shape that is not used as a Drawing Shape, e.g.
     * a text shape that is used as main part of the document as described in
     * OpenDocument 2.3.1 Text Documents.
     *
     * Please don't use yet as the design is not yet finished.
     */
    virtual bool saveOdfContext( KoShapeSavingContext & context ) { Q_UNUSED( context ); return true; }

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
    double scaleX() const;

    /**
     * Return the current scaling adjustment over the Y axis.
     */
    double scaleY() const;

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
    double rotation() const;

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
     * Return the current horizontal shearing angle for this shape.
     * @return the current horizontal shearing angle for this shape.
     */
    double shearX() const;

    /**
     * Return the current vertical shearing angle for this shape.
     * @return the current vertical shearing angle for this shape.
     */
    double shearY() const;

    /**
     * @brief Resize the shape
     *
     * @param size the new size of the shape.  This is different from scaling as
     * scaling is a so called secondary operation which is comparable to zooming in
     * instead of changing the size of the basic shape.
     * Easiest example of this difference is that using this method will not distort the
     * size of pattern-fills and borders.
     */
    virtual void resize( const QSizeF &size );

    /**
     * @brief Get the size of the shape in pt.
     *
     * The size is in shape coordinates.
     *
     * @return the size of the shape as set by resize()
     */
    virtual QSizeF size() const;

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
    virtual QPointF position() const;

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
    virtual QRectF boundingRect() const;

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
    void addConnectionPoint( const QPointF &point );

    /**
     * Return a list of the connectors that have been added to this shape.
     * Note that altering the list or the points in there will not have any
     * effect on the shape.
     * @return a list of the connectors that have been added to this shape.
     */
    QList<QPointF> connectors() const;

    /**
     * Set the background of the shape.
     * A QBrush can have a plain color, be fully transparent or have a complex fill.
     * setting such a brush will allow the shape to fill itself using that brush and
     * will be able to tell if its transparent or not.
     * @param brush the brush for the background.
     */
    void setBackground ( const QBrush & brush );

    /**
     * return the brush used to paint te background of this shape with.
     * A QBrush can have a plain color, be fully transparent or have a complex fill.
     * setting such a brush will allow the shape to fill itself using that brush and
     * will be able to tell if its transparent or not.
     * @return the background-brush
     */
    const QBrush& background ();

    /**
     * Returns true if there is some transparency, false if the shape is fully opaque.
     * The default implementation will just return if the background has some transparency,
     * you should override it and always return true if your shape is not square.
     * @return if the shape is (partly) transparent.
     */
    virtual bool hasTransparency();

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
     * The layering if two overlapping objects have the same index is implementation dependent
     * and probably depends on the order in which they are added to the shape manager.
     * @param zIndex the new z-index;
     */
    void setZIndex(int zIndex);

    /**
     * Changes the Shape to be visible or invisible.
     * Being visible means being painted and printed, as well as being used for
     *   things like guidelines or searches.
     * @param on when true; set the shape to be visible.
     */
    void setVisible(bool on);
    /**
     * Returns current visibility state of this shape.
     * Being visible means being painted and printed, as well as being used for
     *   things like guidelines or searches.
     * @return current visibility state of this shape.
     */
    bool isVisible() const;

    /**
     * Makes it possible for the user to select this shape.
     * This parameter defaults to true.
     * @param selectable when true; set the shape to be selectable.
     */
    void setSelectable(bool selectable);
    /**
     * Returns wheather this shape can be selected by the user.
     * @return true only when the object is selectable.
     */
    bool isSelectable() const;

    /**
     * Changes the Shape to be locked in place.
     * Being locked means the shape can no longer change shape or position.
     * @param locked when true; set the shape to be locked.
     */
    void setLocked(bool locked);
    /**
     * Returns current locked state of this shape.
     * Being locked means the shape can no longer change shape or position.
     * @return current locked state of this shape.
     */
    bool isLocked() const;

    /**
     * Returns the parent, or 0 if there is no parent.
     * @return the parent, or 0 if there is no parent.
     */
    KoShapeContainer *parent() const;

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
    void repaint(const QRectF &shape) const;

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
    void recalcMatrix();

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
    KoShapeBorderModel *border() const;

    /**
     * Set a new border, removing the old one.
     * @param border the new border, or 0 if there should be no border.
     */
    void setBorder(KoShapeBorderModel *border);

    /**
     * Return the insets of the border.
     * Convenience method for KoShapeBorderModel::borderInsets()
     */
    KoInsets borderInsets() const;

    /**
     * Setting the shape to keep its aspect-ratio has the effect that user-scaling will
     * keep the width/hight ratio intact so as not to distort shapes that rely on that
     * ratio.
     * @param keepAspect the new value
     */
    void setKeepAspectRatio(bool keepAspect);

    /**
     * Setting the shape to keep its aspect-ratio has the effect that user-scaling will
     * keep the width/hight ratio intact so as not to distort shapes that rely on that
     * ratio.
     * @return whether to keep aspect ratio of this shape
     */
    bool keepAspectRatio() const;

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

    /**
     * Move this shape from its current (absolute) position over a specified distance.
     * This takes the position of the shape, and moves it in the normal plain. This takes
     * into account the rotation of the object so distanceX really will be the resulting
     * horizontal distance.
     * @param distanceX the horizontal distance to move
     * @param distanceY the vertical distance to move
     */
    void moveBy(double distanceX, double distanceY);

    /**
     * Set a data object on the shape to be used by an application.
     * This is specifically useful when a shape is created in a plugin and that data from that
     * shape should be accessible outside the plugin.
     * @param userData the new user data, or 0 to delete the current one.
     */
    void setUserData(KoShapeUserData *userData);
    /**
     * Return the current userData.
     */
    KoShapeUserData *userData() const;

    /**
     * Set a data object on the shape to be used by an application.
     * This is specifically useful when an application wants to have data that is per shape
     * and should be deleted when the shape is destructed.
     * @param applicationData the new application data, or 0 to delete the current one.
     */
    void setApplicationData(KoShapeApplicationData *applicationData);
    /**
     * Return the current applicationData.
     */
    KoShapeApplicationData *applicationData() const;

    /**
     * Return the Id of this shape, indentifying the type of shape by the id of the factory.
     * @see KoShapeFactory::shapeId()
     * @return the id of the shape-type
     */
    const QString & shapeId() const;
    /**
     * Set the Id of this shape.  A shapeFactory is expected to set the Id at creation
     * so applications can find out what kind of shape this is.
     * @see KoShapeFactory::shapeId()
     * @param id the ID from the factory that created this shape
     */
    void setShapeId(const QString &id);

    /**
     * Create a matrix that describes all the transformations done on this shape.
     * @param converter if not null, this method uses the converter to mark the right
     *        offsets in the current view.
     */
    QMatrix transformationMatrix(const KoViewConverter *converter) const;

    /**
     * Copy all the settings from the parameter shape and apply them to this shape.
     * Settings like the position and rotation to visible and locked.  The parent
     * is a notable exclusion.
     * @param shape the shape to use as original
     */
    virtual void copySettings(const KoShape *shape);

    /**
     * Convenience method that allows people implementing paint() to use the shape
     * internal coordinate system directly to paint itself instead of considering the
     * views zoom.
     * @param painter the painter to alter the zoom level of.
     * @param converter the converter for the current views zoom.
     */
    static void applyConversion(QPainter &painter, const KoViewConverter &converter);

protected:
    /**
     * @brief Get the tag name used for saving
     *
     * Get the name of the tag used for saving drawing shape
     *
     * This will be a pure virtual function once all shapes implemented it.
     *
     * @return the name of the tag
     *
     * @see saveOdf()
     */
    virtual const char * odfTagName() const { return ""; }
    
    /**
     * @brief Save the data that is special by each shape
     *
     * This will be a pure virtual function once all shapes implemented it.
     *
     * @return true if successful, false otherwise
     *
     * @see saveOdf()
     */
    virtual bool saveOdfData( KoShapeSavingContext &context ) const { Q_UNUSED( context ); return true; }

    /**
     * @brief Fill the style object
     *
     * @param style object
     * @param context used for saving
     *
     * @see saveOdf()
     */
    void fillStyle( KoGenStyle &style, KoShapeSavingContext &context ); 

    /**
     * Update the position of the shape in the tree of the KoShapeManager.
     */
    void notifyChanged();

    /// Used by shapeChanged() to select which change was made
    enum ChangeType {
        PositionChanged, ///< used after a setPosition()
        RotationChanged, ///< used after a rotate()
        ScaleChanged,   ///< used after a scale()
        ShearChanged,   ///< used after a shear()
        SizeChanged,    ///< used after a resize()
        ParentChanged,   ///< used after a setParent()
        CollisionDetected ///< used when another shape moved in our boundingrect
    };

    /**
     * A hook that allows inheriting classes to do something after a KoShape property changed
     * This is called whenever the shape, position rotation or scale properties were altered.
     * @param type an indicator which type was changed.
     */
    virtual void shapeChanged(ChangeType type) { Q_UNUSED(type); }

    /**
     * Set the property collision detection.
     * Setting this to true will result in calls to shapeChanged() with the CollisionDetected
     * parameter whenever either this or another shape is moved/rotated etc and intersects this shape.
     * @param detect if true detect collisions.
     */
    void setCollisionDetection(bool detect);
    /**
     * get the property collision detection.
     * @returns true if collision detection is on.
     */
    bool collisionDetection();

    /// return the current matrix that contains the rotation/scale/position of this shape
    const QMatrix& matrix() const;

private:
    /**
     * @brief Get the style used for the shape
     *
     * This method calls fillStyle and add then the style to the context
     *
     * @param context used for saving
     * @return the name of the style
     */
    QString getStyle( KoShapeSavingContext &context );

    friend class KoShapeManager;
    void addShapeManager( KoShapeManager * manager );
    void removeShapeManager( KoShapeManager * manager );

    KoShapePrivate * const d;
};

#endif
