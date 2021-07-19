/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006, 2008 C. Boemann <cbo@boemann.dk>
   SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2007-2009, 2011 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSHAPE_H
#define KOSHAPE_H

#include "KoFlake.h"
#include "KoFlakeTypes.h"

#include <QSharedPointer>
#include <QSet>
#include <QMap>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QTransform>

#include <QDomDocument>

#include "kritaflake_export.h"

class QPainter;
class QRectF;
class QPainterPath;
class QTransform;

class KoShapeContainer;
class KoShapeStrokeModel;
class KoShapeUserData;
class KoViewConverter;
class KoShapeApplicationData;
class KoShapeSavingContext;
class KoShapeLoadingContext;
class KoShapeShadow;
class KoFilterEffectStack;
class KoSnapData;
class KoClipPath;
class KoClipMask;
class KoShapePaintingContext;
class KoShapeAnchor;
struct KoInsets;
class KoShapeBackground;
class KisHandlePainterHelper;
class KoShapeManager;

/**
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
 *
 * <h1>Shape interaction notifications</h1>
 * We had several notification methods that allow your shape to be notified of changes in other
 * shapes positions or rotation etc.
 * <ol><li>The most general is KoShape::shapeChanged().<br>
 * a virtual method that you can use to check various changed to your shape made by tools or otherwise.</li>
 * <li>for shape hierarchies the parent may receive a notification when a child was modified.
 *  This is done though KoShapeContainerModel::childChanged()</li>
 * <li>any shape that is at a similar position as another shape there is collision detection.
 * You can register your shape to be sensitive to any changes like moving or whatever to
 * <b>other</b> shapes that intersect yours.
 * Such changes will then be notified to your shape using the method from (1) You should call
 * KoShape::setCollisionDetection(bool) to enable this.
 * </ol>
 */
class KRITAFLAKE_EXPORT KoShape
{
public:
    /// Used by shapeChanged() to select which change was made
    enum ChangeType {
        PositionChanged, ///< used after a setPosition()
        RotationChanged, ///< used after a setRotation()
        ScaleChanged,   ///< used after a scale()
        ShearChanged,   ///< used after a shear()
        SizeChanged,    ///< used after a setSize()
        GenericMatrixChange,    ///< used after the matrix was changed without knowing which property explicitly changed
        KeepAspectRatioChange, ///< used after setKeepAspectRatio()
        ParentChanged,   ///< used after a setParent()
        Deleted, ///< the shape was deleted
        StrokeChanged, ///< the shapes stroke has changed
        BackgroundChanged, ///< the shapes background has changed
        ShadowChanged, ///< the shapes shadow has changed
        BorderChanged, ///< the shapes border has changed
        ParameterChanged, ///< the shapes parameter has changed (KoParameterShape only)
        ContentChanged, ///< the content of the shape changed e.g. a new image inside a pixmap/text change inside a textshape
        TextRunAroundChanged, ///< used after a setTextRunAroundSide()
        ChildChanged, ///< a child of a container was changed/removed. This is propagated to all parents
        ConnectionPointChanged, ///< a connection point has changed
        ClipPathChanged, ///< the shapes clip path has changed
        ClipMaskChanged, ///< the shapes clip path has changed
        TransparencyChanged ///< the shapetransparency value has changed
    };

    /// The behavior text should do when intersecting this shape.
    enum TextRunAroundSide {
        BiggestRunAroundSide,   ///< Run other text around the side that has the most space
        LeftRunAroundSide,      ///< Run other text around the left side of the frame
        RightRunAroundSide,     ///< Run other text around the right side of the frame
        EnoughRunAroundSide,      ///< Run other text dynamically around both sides of the shape, provided there is sufficient space left
        BothRunAroundSide,      ///< Run other text around both sides of the shape
        NoRunAround,            ///< The text will be completely avoiding the frame by keeping the horizontal space that this frame occupies blank.
        RunThrough              ///< The text will completely ignore the frame and layout as if it was not there
    };

    /// The behavior text should do when intersecting this shape.
    enum TextRunAroundContour {
        ContourBox,     /// Run other text around a bounding rect of the outline
        ContourFull,   ///< Run other text around also on the inside
        ContourOutside   ///< Run other text around only on the outside
    };

    /**
     * TODO
     */
    enum RunThroughLevel {
        Background,
        Foreground
    };

    /**
     * @brief Constructor
     */
    KoShape();

    /**
     * @brief Destructor
     */
    virtual ~KoShape();

    /**
     * @brief creates a deep copy of the shape or shape's subtree
     * @return a cloned shape
     */
    virtual KoShape* cloneShape() const;

    /**
     * @brief Paint the shape fill
     * The class extending this one is responsible for painting itself. \p painter is expected
     * to be preconfigured to work in "document" pixels.
     *
     * @param painter used for painting the shape
     * @param paintcontext the painting context.
     */
    virtual void paint(QPainter &painter, KoShapePaintingContext &paintcontext) const = 0;

    /**
     * @brief paintStroke paints the shape's stroked outline
     * @param painter used for painting the shape
     * @see applyConversion()
     * @param paintcontext the painting context.
     */
    virtual void paintStroke(QPainter &painter, KoShapePaintingContext &paintcontext) const;

    /**
     * @brief Scale the shape using the zero-point which is the top-left corner.
     * @see position()
     *
     * @param sx scale in x direction
     * @param sy scale in y direction
     */
    void scale(qreal sx, qreal sy);

    /**
     * @brief Rotate the shape (relative)
     *
     * The shape will be rotated from the current rotation using the center of the shape using the size()
     *
     * @param angle change the angle of rotation increasing it with 'angle' degrees
     */
    void rotate(qreal angle);

    /**
     * Return the current rotation in degrees.
     * It returns NaN if the shape has a shearing or scaling transformation applied.
     */
    qreal rotation() const;

    /**
     * @brief Shear the shape
     * The shape will be sheared using the zero-point which is the top-left corner.
     * @see position()
     *
     * @param sx shear in x direction
     * @param sy shear in y direction
     */
    void shear(qreal sx, qreal sy);

    /**
     * @brief Resize the shape
     *
     * @param size the new size of the shape.  This is different from scaling as
     * scaling is a so called secondary operation which is comparable to zooming in
     * instead of changing the size of the basic shape.
     * Easiest example of this difference is that using this method will not distort the
     * size of pattern-fills and strokes.
     */
    virtual void setSize(const QSizeF &size);

    /**
     * @brief Get the size of the shape in pt.
     *
     * The size is in shape coordinates.
     *
     * @return the size of the shape as set by setSize()
     */
    virtual QSizeF size() const;

    /**
     * @brief Set the position of the shape in pt
     *
     * @param position the new position of the shape
     */
    virtual void setPosition(const QPointF &position);

    /**
     * @brief Get the position of the shape in pt
     *
     * @return the position of the shape
     */
    QPointF position() const;

    /**
     * @brief Check if the shape is hit on position
     * @param position the position where the user clicked.
     * @return true when it hits.
     */
    virtual bool hitTest(const QPointF &position) const;

    /**
     * @brief Get the bounding box of the shape
     *
     * This includes the line width and the shadow of the shape
     *
     * @return the bounding box of the shape
     */
    virtual QRectF boundingRect() const;

    /**
     * Get the united bounding box of a group of shapes. This is a utility
     * function used in many places in Krita.
     */
    static QRectF boundingRect(const QList<KoShape*> &shapes);

    /**
     * @return the bounding rect of the outline of the shape measured
     *         in absolute coordinate system. Please note that in contrast to
     *         boundingRect() this rect doesn't include the stroke and other
     *         insets.
     */
    QRectF absoluteOutlineRect() const;

    /**
     * Same as a member function, but applies to a list of shapes and returns a
     * united rect.
     */
    static QRectF absoluteOutlineRect(const QList<KoShape*> &shapes);

    /**
     * Return the side text should flow around this shape. This implements the ODF style:wrap
     * attribute that specifies how text is displayed around a frame or graphic object.
     */
    TextRunAroundSide textRunAroundSide() const;

    /**
     * Set the side text should flow around this shape.
     * @param side the requested side
     * @param runThrough run through the foreground or background or...
     */
    void setTextRunAroundSide(TextRunAroundSide side, RunThroughLevel runThrough = Background);

    /**
     * The space between this shape's left edge and text that runs around this shape.
     * @return the space around this shape to keep free from text
     */
    qreal textRunAroundDistanceLeft() const;

    /**
     * Set the space between this shape's left edge and the text that run around this shape.
     * @param distance the space around this shape to keep free from text
     */
    void setTextRunAroundDistanceLeft(qreal distance);

    /**
     * The space between this shape's top edge and text that runs around this shape.
     * @return the space around this shape to keep free from text
     */
    qreal textRunAroundDistanceTop() const;

    /**
     * Set the space between this shape's top edge and the text that run around this shape.
     * @param distance the space around this shape to keep free from text
     */
    void setTextRunAroundDistanceTop(qreal distance);

    /**
     * The space between this shape's right edge and text that runs around this shape.
     * @return the space around this shape to keep free from text
     */
    qreal textRunAroundDistanceRight() const;

    /**
     * Set the space between this shape's right edge and the text that run around this shape.
     * @param distance the space around this shape to keep free from text
     */
    void setTextRunAroundDistanceRight(qreal distance);

    /**
     * The space between this shape's bottom edge and text that runs around this shape.
     * @return the space around this shape to keep free from text
     */
    qreal textRunAroundDistanceBottom() const;

    /**
     * Set the space between this shape's bottom edge and the text that run around this shape.
     * @param distance the space around this shape to keep free from text
     */
    void setTextRunAroundDistanceBottom(qreal distance);

    /**
     * Return the threshold above which text should flow around this shape.
     * The text will not flow around the shape on a side unless the space available on that side
     * is above this threshold. Only used when the text run around side is EnoughRunAroundSide.
     * @return threshold the threshold
     */
    qreal textRunAroundThreshold() const;

    /**
     * Set the threshold above which text should flow around this shape.
     * The text will not flow around the shape on a side unless the space available on that side
     * is above this threshold. Only used when the text run around side is EnoughRunAroundSide.
     * @param threshold the new threshold
     */
    void setTextRunAroundThreshold(qreal threshold);

    /**
     * Return the how tight text run around is done around this shape.
     * @return the contour
     */
    TextRunAroundContour textRunAroundContour() const;

    /**
     * Set how tight text run around is done around this shape.
     * @param contour the new contour
     */
    void setTextRunAroundContour(TextRunAroundContour contour);

    /**
     * Set the KoShapeAnchor
     */
    void setAnchor(KoShapeAnchor *anchor);

    /**
     * Return the KoShapeAnchor, or 0
     */
    KoShapeAnchor *anchor() const;

    /**
     * Set the minimum height of the shape.
     * Currently it's not respected but only for informational purpose
     * @param height the minimum height of the frame.
     */
    void setMinimumHeight(qreal height);

    /**
     * Return the minimum height of the shape.
     * @return the minimum height of the shape. Default is 0.0.
     */
    qreal minimumHeight() const;


    /**
     * Set the background of the shape.
     * A shape background can be a plain color, a gradient, a pattern, be fully transparent
     * or have a complex fill.
     * Setting such a background will allow the shape to be filled and will be able to tell
     * if it is transparent or not.
     *
     * If the shape inherited the background from its parent, its stops inheriting it, that
     * is inheritBackground property resets to false.
     *
     * @param background the new shape background.
     */
    void setBackground(QSharedPointer<KoShapeBackground> background);

    /**
     * return the brush used to paint te background of this shape with.
     * A QBrush can have a plain color, be fully transparent or have a complex fill.
     * setting such a brush will allow the shape to fill itself using that brush and
     * will be able to tell if its transparent or not.
     * @return the background-brush
     */
    QSharedPointer<KoShapeBackground> background() const;

    /**
     * @brief setInheritBackground marks a shape as inhiriting the background
     * from the parent shape. NOTE: The currently selected background is destroyed.
     * @param value true if the shape should inherit the filling background
     */
    void setInheritBackground(bool value);

    /**
     * @brief inheritBackground shows if the shape inherits background from its parent
     * @return true if the shape inherits the fill
     */
    bool inheritBackground() const;

    /**
     * Returns true if there is some transparency, false if the shape is fully opaque.
     * The default implementation will just return if the background has some transparency,
     * you should override it and always return true if your shape is not square.
     * @return if the shape is (partly) transparent.
     */
    virtual bool hasTransparency() const;

    /**
     * Sets shape level transparency.
     * @param transparency the new shape level transparency
     */
    void setTransparency(qreal transparency);

    /**
     * Returns the shape level transparency.
     * @param recursive when true takes the parents transparency into account
     */
    qreal transparency(bool recursive=false) const;

    /**
     * Retrieve the z-coordinate of this shape.
     * The zIndex property is used to determine which shape lies on top of other objects.
     * An shape with a higher z-order is on top, and can obscure another shape.
     * @return the z-index of this shape.
     * @see setZIndex()
     */
    qint16 zIndex() const;

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
    void setZIndex(qint16 zIndex);

    /**
     * Maximum value of z-index
     */
    static const qint16 maxZIndex;

    /**
     * Minimum value of z-index
     */
    static const qint16 minZIndex;

    /**
     * Retrieve the run through property of this shape.
     * The run through property is used to determine if the shape is behind, inside or before text.
     * @return the run through of this shape.
     */
    int runThrough() const;

    /**
     * Set the run through property of this shape.
     * The run through property is used to determine if the shape is behind, inside or before text.
     * @param runThrough the new run through;
     */
    virtual void setRunThrough(short int runThrough);

    /**
     * Changes the Shape to be visible or invisible.
     * Being visible means being painted, as well as being used for
     *   things like guidelines or searches.
     * @param on when true; set the shape to be visible.
     * @see setGeometryProtected(), setContentProtected(), setSelectable()
     */
    void setVisible(bool on);
    /**
     * Returns current visibility state of this shape.
     * Being visible means being painted, as well as being used for
     *   things like guidelines or searches.
     * @param recursive when true, checks visibility recursively
     * @return current visibility state of this shape.
     * @see isGeometryProtected(), isContentProtected(), isSelectable()
     */
    bool isVisible(bool recursive = true) const;

    /**
     * Changes the shape to be printable or not. The default is true.
     *
     * If a Shape's print flag is true, the shape will be printed. If
     * false, the shape will not be printed. If a shape is not visible (@see isVisible),
     * it isPrinted will return false, too.
     */
    void setPrintable(bool on);

    /**
     * Returns the current printable state of this shape.
     *
     * A shape can be visible but not printable, not printable and not visible
     * or visible and printable, but not invisible and still printable.
     *
     * @return current printable state of this shape.
     */
    bool isPrintable() const;

    /**
     * Makes it possible for the user to select this shape.
     * This parameter defaults to true.
     * @param selectable when true; set the shape to be selectable by the user.
     * @see setGeometryProtected(), setContentProtected(), setVisible()
     */
    void setSelectable(bool selectable);

    /**
     * Returns if this shape can be selected by the user.
     * @return true only when the object is selectable.
     * @see isGeometryProtected(), isContentProtected(), isVisible()
     */
    bool isSelectable() const;

    /**
     * Tells the shape to have its position/rotation and size protected from user-changes.
     * The geometry being protected means the user can not change shape or position of the
     * shape. This includes any matrix operation such as rotation.
     * @param on when true; set the shape to have its geometry protected.
     * @see setContentProtected(), setSelectable(), setVisible()
     */
    void setGeometryProtected(bool on);

    /**
     * Returns current geometry protection state of this shape.
     * The geometry being protected means the user can not change shape or position of the
     * shape. This includes any matrix operation such as rotation.
     * @return current geometry protection state of this shape.
     * @see isContentProtected(), isSelectable(), isVisible()
     */
    bool isGeometryProtected() const;

    /**
     * Marks the shape to have its content protected against editing.
     * Content protection is a hint for tools to disallow the user editing the content.
     * @param protect when true set the shapes content to be protected from user modification.
     * @see setGeometryProtected(), setSelectable(), setVisible()
     */
    void setContentProtected(bool protect);

    /**
     * Returns current content protection state of this shape.
     * Content protection is a hint for tools to disallow the user editing the content.
     * @return current content protection state of this shape.
     * @see isGeometryProtected(), isSelectable(), isVisible()
     */
    bool isContentProtected() const;

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
     * @brief inheritsTransformFromAny checks if the shape inherits transformation from
     *        any of the shapes listed in \p ancestorsInQuestion. The inheritance is checked
     *        in recursive way.
     * @return true if there is a (transitive) transformation-wise parent found in \p ancestorsInQuestion
     */
    bool inheritsTransformFromAny(const QList<KoShape*> ancestorsInQuestion) const;

    /**
     * @return true if this shape has a common parent with \p shape
     */
    bool hasCommonParent(const KoShape *shape) const;

    /**
     * Request a repaint to be queued.
     * The repaint will be of the entire Shape, including its selection handles should this
     * shape be selected.
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     */
    virtual void update() const;

    /**
     * Request a repaint to be queued.
     * The repaint will be restricted to the parameters rectangle, which is expected to be
     * in absolute coordinates of the canvas and it is expected to be
     * normalized.
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     * @param rect the rectangle (in pt) to queue for repaint.
     */
    virtual void updateAbsolute(const QRectF &rect) const;

    /// Used by compareShapeZIndex() to order shapes
    enum ChildZOrderPolicy {
        ChildZDefault,
        ChildZParentChild = ChildZDefault, ///< normal parent/child ordering
        ChildZPassThrough ///< children are considered equal to this shape
    };

   /**
    * Returns if during compareShapeZIndex() how this shape portrays the values
    * of its children. The default behaviour is to let this shape's z values take
    * the place of its childrens values, so you get a parent/child relationship.
    * The children are naturally still ordered relatively to their z values
    *
    * But for special cases (like Calligra's TextShape) it can be overloaded to return
    * ChildZPassThrough which means the children keep their own z values
    * @returns the z order policy of this shape
    */
    virtual ChildZOrderPolicy childZOrderPolicy();

    /**
     * This is a method used to sort a list using the STL sorting methods.
     * @param s1 the first shape
     * @param s2 the second shape
     */
    static bool compareShapeZIndex(KoShape *s1, KoShape *s2);

    /**
     * returns the outline of the shape in the form of a path.
     * The outline returned will always be relative to the position() of the shape, so
     * moving the shape will not alter the result.  The outline is used to draw the stroke
     * on, for example.
     * @returns the outline of the shape in the form of a path.
     */
    virtual QPainterPath outline() const;

    /**
     * returns the outline of the shape in the form of a rect.
     * The outlineRect returned will always be relative to the position() of the shape, so
     * moving the shape will not alter the result.  The outline is used to calculate
     * the boundingRect.
     * @returns the outline of the shape in the form of a rect.
     */
    virtual QRectF outlineRect() const;

    /**
     * returns the outline of the shape in the form of a path for the use of painting a shadow.
     *
     * Normally this would be the same as outline() if there is a fill (background) set on the
     * shape and empty if not.  However, a shape could reimplement this to return an outline
     * even if no fill is defined. A typical example of this would be the picture shape
     * which has a picture but almost never a background.
     *
     * @returns the outline of the shape in the form of a path.
     */
    virtual QPainterPath shadowOutline() const;

    /**
     * Returns the currently set stroke, or 0 if there is no stroke.
     * @return the currently set stroke, or 0 if there is no stroke.
     */
    KoShapeStrokeModelSP stroke() const;

    /**
     * Set a new stroke, removing the old one. The stroke inheritance becomes disabled.
     * @param stroke the new stroke, or 0 if there should be no stroke.
     */
    void setStroke(KoShapeStrokeModelSP stroke);

    /**
     * @brief setInheritStroke marks a shape as inhiriting the stroke
     * from the parent shape. NOTE: The currently selected stroke is destroyed.
     * @param value true if the shape should inherit the stroke style
     */
    void setInheritStroke(bool value);

    /**
     * @brief inheritStroke shows if the shape inherits the stroke from its parent
     * @return true if the shape inherits the stroke style
     */
    bool inheritStroke() const;

    /**
     * Return the insets of the stroke.
     * Convenience method for KoShapeStrokeModel::strokeInsets()
     */
    KoInsets strokeInsets() const;

    /// Sets the new shadow, removing the old one
    void setShadow(KoShapeShadow *shadow);

    /// Returns the currently set shadow or 0 if there is no shadow set
    KoShapeShadow *shadow() const;

    /// Sets a new clip path, removing the old one
    void setClipPath(KoClipPath *clipPath);

    /// Returns the currently set clip path or 0 if there is no clip path set
    KoClipPath * clipPath() const;

    /// Sets a new clip mask, removing the old one. The mask is owned by the shape.
    void setClipMask(KoClipMask *clipMask);

    /// Returns the currently set clip mask or 0 if there is no clip mask set
    KoClipMask* clipMask() const;

    /**
     * Setting the shape to keep its aspect-ratio has the effect that user-scaling will
     * keep the width/height ratio intact so as not to distort shapes that rely on that
     * ratio.
     * @param keepAspect the new value
     */
    void setKeepAspectRatio(bool keepAspect);

    /**
     * Setting the shape to keep its aspect-ratio has the effect that user-scaling will
     * keep the width/height ratio intact so as not to distort shapes that rely on that
     * ratio.
     * @return whether to keep aspect ratio of this shape
     */
    bool keepAspectRatio() const;

    /**
     * Return the position of this shape regardless of rotation/skew/scaling and regardless of
     * this shape having a parent (being in a group) or not.<br>
     * @param anchor The place on the (unaltered) shape that you want the position of.
     * @return the point that is the absolute, centered position of this shape.
     */
    QPointF absolutePosition(KoFlake::AnchorPosition anchor = KoFlake::Center) const;

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
     * @param anchor The place on the (unaltered) shape that you set the position of.
     */
    void setAbsolutePosition(const QPointF &newPosition, KoFlake::AnchorPosition anchor = KoFlake::Center);

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
     * Return the Id of this shape, identifying the type of shape by the id of the factory.
     * @see KoShapeFactoryBase::shapeId()
     * @return the id of the shape-type
     */
    QString shapeId() const;

    /**
     * Set the Id of this shape.  A shapeFactory is expected to set the Id at creation
     * so applications can find out what kind of shape this is.
     * @see KoShapeFactoryBase::shapeId()
     * @param id the ID from the factory that created this shape
     */
    void setShapeId(const QString &id);

    /**
     * Create a matrix that describes all the transformations done on this shape.
     *
     * The absolute transformation is the combined transformation of this shape
     * and all its parents and grandparents.
     */
    QTransform absoluteTransformation() const;

    /**
     * Applies a transformation to this shape.
     *
     * The transformation given is relative to the global coordinate system, i.e. the document.
     * This is a convenience function to apply a global transformation to this shape.
     * @see applyTransformation
     *
     * @param matrix the transformation matrix to apply
     */
    void applyAbsoluteTransformation(const QTransform &matrix);

    /**
     * Sets a new transformation matrix describing the local transformations on this shape.
     * @param matrix the new transformation matrix
     */
    void setTransformation(const QTransform &matrix);

    /// Returns the shapes local transformation matrix
    QTransform transformation() const;

    /**
     * Applies a transformation to this shape.
     *
     * The transformation given is relative to the shape coordinate system.
     *
     * @param matrix the transformation matrix to apply
     */
    void applyTransformation(const QTransform &matrix);

    /**
     * Copy all the settings from the parameter shape and apply them to this shape.
     * Settings like the position and rotation to visible and locked.  The parent
     * is a notable exclusion.
     * @param shape the shape to use as original
     */
    void copySettings(const KoShape *shape);

    /**
     * A convenience method that creates a handles helper with applying transformations at
     * the same time. Please note that you shouldn't save/restore additionally. All the work
     * on restoring original painter's transformations is done by the helper.
     */
    static KisHandlePainterHelper createHandlePainterHelperView(QPainter *painter, KoShape *shape, const KoViewConverter &converter, qreal handleRadius = 0.0);
    static KisHandlePainterHelper createHandlePainterHelperDocument(QPainter *painter, KoShape *shape, qreal handleRadius);

    /**
     * @brief Transforms point from shape coordinates to document coordinates
     * @param point in shape coordinates
     * @return point in document coordinates
     */
    QPointF shapeToDocument(const QPointF &point) const;

    /**
     * @brief Transforms rect from shape coordinates to document coordinates
     * @param rect in shape coordinates
     * @return rect in document coordinates
     */
    QRectF shapeToDocument(const QRectF &rect) const;

    /**
     * @brief Transforms point from document coordinates to shape coordinates
     * @param point in document coordinates
     * @return point in shape coordinates
     */
    QPointF documentToShape(const QPointF &point) const;

    /**
     * @brief Transform rect from document coordinates to shape coordinates
     * @param rect in document coordinates
     * @return rect in shape coordinates
     */
    QRectF documentToShape(const QRectF &rect) const;

    /**
     * Returns the name of the shape.
     * @return the shapes name
     */
    QString name() const;

    /**
     * Sets the name of the shape.
     * @param name the new shape name
     */
    void setName(const QString &name);

    /**
     * Update the position of the shape in the tree of the KoShapeManager.
     */
    void notifyChanged();

    /**
     * A shape can be in a state that it is doing processing data like loading or text layout.
     * In this case it can be shown on screen probably partially but it should really not be printed
     * until it is fully done processing.
     * Warning! This method can be blocking for a long time
     * @param asynchronous If set to true the processing will can take place in a different thread and the
     *                     function will not block until the shape is finished.
     *                     In case of printing Flake will call this method from a non-main thread and only
     *                     start printing it when the in case of printing method returned.
     *                     If set to false the processing needs to be done synchronously and will
     *                     block until the result is finished.
     */
    virtual void waitUntilReady(bool asynchronous = true) const;

    /// checks recursively if the shape or one of its parents is not visible or locked
    virtual bool isShapeEditable(bool recursive = true) const;

    /**
     * Adds a shape which depends on this shape.
     * Making a shape dependent on this one means it will get shapeChanged() called
     * on each update of this shape.
     *
     * If this shape already depends on the given shape, establishing the
     * dependency is refused to prevent circular dependencies.
     *
     * @param shape the shape which depends on this shape
     * @return true if dependency could be established, otherwise false
     * @see removeDependee(), hasDependee()
     */
    bool addDependee(KoShape *shape);

    /**
     * Removes as shape depending on this shape.
     * @see addDependee(), hasDependee()
     */
    void removeDependee(KoShape *shape);

    /// Returns if the given shape is dependent on this shape
    bool hasDependee(KoShape *shape) const;

    /// Returns list of shapes depending on this shape
    QList<KoShape*> dependees() const;

    /// Returns additional snap data the shape wants to have snapping to
    virtual KoSnapData snapData() const;

    /**
     * Set additional attribute
     *
     * This can be used to attach additional attributes to a shape for attributes
     * that are application specific like presentation:placeholder
     *
     * @param name The name of the attribute in the following form prefix:tag e.g. presentation:placeholder
     * @param value The value of the attribute
     */
    void setAdditionalAttribute(const QString &name, const QString &value);

    /**
     * Remove additional attribute
     *
     * @param name The name of the attribute in the following form prefix:tag e.g. presentation:placeholder
     */
    void removeAdditionalAttribute(const QString &name);

    /**
     * Check if additional attribute is set
     *
     * @param name The name of the attribute in the following form prefix:tag e.g. presentation:placeholder
     *
     * @return true if there is a attribute with prefix:tag set, false otherwise
     */
    bool hasAdditionalAttribute(const QString &name) const;

    /**
     * Get additional attribute
     *
     * @param name The name of the attribute in the following form prefix:tag e.g. presentation:placeholder
     *
     * @return The value of the attribute if it exists or a null string if not found.
     */
    QString additionalAttribute(const QString &name) const;

    void setAdditionalStyleAttribute(const char *name, const QString &value);

    void removeAdditionalStyleAttribute(const char *name);

    /**
     * Returns the filter effect stack of the shape
     *
     * @return the list of filter effects applied on the shape when rendering.
     */
    KoFilterEffectStack *filterEffectStack() const;

    /// Sets the new filter effect stack, removing the old one
    void setFilterEffectStack(KoFilterEffectStack *filterEffectStack);

    /**
     * Return the tool delegates for this shape.
     * In Flake a shape being selected will cause the tool manager to make available all tools that
     * can edit the selected shapes.  In some cases selecting one shape should allow the tool to
     * edit a related shape be available too.  The tool delegates allows this to happen by taking
     * all the shapes in the set into account on tool selection.
     * Notice that if the set is non-empty 'this' shape is no longer looked at. You can choose
     * to add itself to the set too.
     */
    QSet<KoShape*> toolDelegates() const;

    /**
     * Set the tool delegates.
     * @param delegates the new delegates.
     * @see toolDelegates()
     */
    void setToolDelegates(const QSet<KoShape*> &delegates);

    /**
     * Return the hyperlink for this shape.
     */
    QString hyperLink () const;

    /**
     * Set hyperlink for this shape.
     * @param hyperLink name.
     */
    void setHyperLink(const QString &hyperLink);

    void setAbsolute(bool);

public:

    struct KRITAFLAKE_EXPORT ShapeChangeListener {
        virtual ~ShapeChangeListener();
        virtual void notifyShapeChanged(ChangeType type, KoShape *shape) = 0;

    private:
        friend class KoShape;
        void registerShape(KoShape *shape);
        void unregisterShape(KoShape *shape);
        void notifyShapeChangedImpl(ChangeType type, KoShape *shape);

        QList<KoShape*> m_registeredShapes;
    };

    void addShapeChangeListener(ShapeChangeListener *listener);
    void removeShapeChangeListener(ShapeChangeListener *listener);

protected:
    QList<ShapeChangeListener *> listeners() const;
    void setSizeImpl(const QSizeF &size) const;

public:
    static QList<KoShape*> linearizeSubtree(const QList<KoShape*> &shapes);
    static QList<KoShape *> linearizeSubtreeSorted(const QList<KoShape *> &shapes);
protected:
    KoShape(const KoShape &rhs);

     /**
     * A hook that allows inheriting classes to do something after a KoShape property changed
     * This is called whenever the shape, position rotation or scale properties were altered.
     * @param type an indicator which type was changed.
     * @param shape the shape.
     */
    virtual void shapeChanged(ChangeType type, KoShape *shape = 0);

    /// return the current matrix that contains the rotation/scale/position of this shape
    QTransform transform() const;

private:
    class Private;
    QScopedPointer<Private> d;

    class SharedData;
    QSharedDataPointer<SharedData> s;

    bool absolute;

protected:
    /**
     * Notify the shape that a change was done. To be used by inheriting shapes.
     * @param type the change type
     */
    void shapeChangedPriv(KoShape::ChangeType type);

private:
    void addShapeManager(KoShapeManager *manager);
    void removeShapeManager(KoShapeManager *manager);
    friend class KoShapeManager;
};

Q_DECLARE_METATYPE(KoShape*)

#endif
