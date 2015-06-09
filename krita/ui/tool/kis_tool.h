/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_H_
#define KIS_TOOL_H_

#include <QCursor>

#include <KoColor.h>
#include <KoToolBase.h>
#include <KoID.h>
#include <KoCanvasResourceManager.h>
#include <krita_export.h>
#include <kis_types.h>


#define PRESS_CONDITION(_event, _mode, _button, _modifier)              \
    (this->mode() == (_mode) && (_event)->button() == (_button) &&            \
     (_event)->modifiers() == (_modifier))

#define PRESS_CONDITION_WB(_event, _mode, _button, _modifier)            \
    (this->mode() == (_mode) && (_event)->button() & (_button) &&            \
     (_event)->modifiers() == (_modifier))

#define PRESS_CONDITION_OM(_event, _mode, _button, _modifier)           \
    (this->mode() == (_mode) && (_event)->button() == (_button) &&      \
     ((_event)->modifiers() & (_modifier) ||                            \
      (_event)->modifiers() == Qt::NoModifier))

#define RELEASE_CONDITION(_event, _mode, _button)               \
    (this->mode() == (_mode) && (_event)->button() == (_button))

#define RELEASE_CONDITION_WB(_event, _mode, _button)               \
    (this->mode() == (_mode) && (_event)->button() & (_button))

#define MOVE_CONDITION(_event, _mode) (this->mode() == (_mode))

#ifdef __GNUC__
#define WARN_WRONG_MODE(_mode) qWarning() << "Unexpected tool event has come to" << __func__ << "while being mode" << _mode << "!"
#else
#define WARN_WRONG_MODE(_mode) qWarning() << "Unexpected tool event has come while being mode" << _mode << "!"
#endif

#define CHECK_MODE_SANITY_OR_RETURN(_mode) if (mode() != _mode) { WARN_WRONG_MODE(mode()); return; }

class KoCanvasBase;
class KoPattern;
class KoAbstractGradient;
class KisFilterConfiguration;
class QPainter;
class QPainterPath;
class QPolygonF;
class KisRecordedPaintAction;

/// Definitions of the toolgroups of Krita
static const QString TOOL_TYPE_SHAPE = "0 Krita/Shape";         // Geometric shapes like ellipses and lines
static const QString TOOL_TYPE_FREEHAND = "1 Krita/Freehand";   // Freehand drawing tools
static const QString TOOL_TYPE_TRANSFORM = "2 Krita/Transform"; // Tools that transform the layer;
static const QString TOOL_TYPE_FILL = "3 Krita/Fill";                // Tools that fill parts of the canvas
static const QString TOOL_TYPE_VIEW = "4 Krita/View";                // Tools that affect the canvas: pan, zoom, etc.
static const QString TOOL_TYPE_SELECTED = "5 Krita/Select";          // Tools that select pixels

//activation id for Krita tools, Krita tools are always active and handle locked and invisible layers by themself
static const QString KRITA_TOOL_ACTIVATION_ID = "flake/always";

class  KRITAUI_EXPORT KisTool
        : public KoToolBase
{
    Q_OBJECT

    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    enum { FLAG_USES_CUSTOM_PRESET=0x01, FLAG_USES_CUSTOM_COMPOSITEOP=0x02 };

    KisTool(KoCanvasBase * canvas, const QCursor & cursor);
    virtual ~KisTool();

    virtual int flags() const { return 0; }

    void deleteSelection();
// KoToolBase Implementation.

public:

    /**
     * Called by KisToolProxy when the primary action of the tool is
     * going to be started now, that is when all the modifiers are
     * pressed and the only thing left is just to press the mouse
     * button.  On coming of this callback the tool is supposed to
     * prepare the cursor and/or the outline to show the user shat is
     * going to happen next
     */
    virtual void activatePrimaryAction();

    /**
     * Called by KisToolProxy when the primary is no longer possible
     * to be started now, e.g. when its modifiers and released. The
     * tool is supposed revert all the preparetions it has doen in
     * activatePrimaryAction().
     */
    virtual void deactivatePrimaryAction();

    /**
     * Called by KisToolProxy when a primary action for the tool is
     * started. The \p event stores the original event that
     * started the stroke. The \p event is _accepted_ by default. If
     * the tool decides to ignore this particular action (e.g. when
     * the node is not editable), it should call event->ignore(). Then
     * no further continuePrimaryAction() or endPrimaryAction() will
     * be called until the next user action.
     */
    virtual void beginPrimaryAction(KoPointerEvent *event);

    /**
     * Called by KisToolProxy when the primary action is in progress
     * of pointer movement.  If the tool has ignored the event in
     * beginPrimaryAction(), this method will not be called.
     */
    virtual void continuePrimaryAction(KoPointerEvent *event);

    /**
     * Called by KisToolProxy when the primary action is being
     * finished, that is while mouseRelease or tabletRelease event.
     * If the tool has ignored the event in beginPrimaryAction(), this
     * method will not be called.
     */
    virtual void endPrimaryAction(KoPointerEvent *event);

    /**
     * The same as beginPrimaryAction(), but called when the stroke is
     * started by a double-click
     *
     * \see beginPrimaryAction()
     */
    virtual void beginPrimaryDoubleClickAction(KoPointerEvent *event);

    /**
     * Returns true if the tool can handle (and wants to handle) a
     * very tight flow of input events from the tablet
     */
    virtual bool primaryActionSupportsHiResEvents() const;

    enum ToolAction {
        Primary,
        AlternateChangeSize,
        AlternatePickFgNode,
        AlternatePickBgNode,
        AlternatePickFgImage,
        AlternatePickBgImage,
        AlternateSecondary,
        AlternateThird,
        AlternateFourth,
        AlternateFifth,
        Alternate_NONE = 10000
    };

    enum AlternateAction {
        ChangeSize = AlternateChangeSize,
        PickFgNode = AlternatePickFgNode,
        PickBgNode = AlternatePickBgNode,
        PickFgImage = AlternatePickFgImage,
        PickBgImage = AlternatePickBgImage,
        Secondary = AlternateSecondary,
        Third = AlternateThird,
        Fourth = AlternateFourth,
        Fifth = AlternateFifth,
        NONE = 10000
    };

    static AlternateAction actionToAlternateAction(ToolAction action);

    virtual void activateAlternateAction(AlternateAction action);
    virtual void deactivateAlternateAction(AlternateAction action);

    virtual void beginAlternateAction(KoPointerEvent *event, AlternateAction action);
    virtual void continueAlternateAction(KoPointerEvent *event, AlternateAction action);
    virtual void endAlternateAction(KoPointerEvent *event, AlternateAction action);
    virtual void beginAlternateDoubleClickAction(KoPointerEvent *event, AlternateAction action);


    void mousePressEvent(KoPointerEvent *event);
    void mouseDoubleClickEvent(KoPointerEvent *event);
    void mouseTripleClickEvent(KoPointerEvent *event);
    void mouseReleaseEvent(KoPointerEvent *event);
    void mouseMoveEvent(KoPointerEvent *event);

    bool isActive() const;

public Q_SLOTS:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();
    virtual void canvasResourceChanged(int key, const QVariant & res);
    // Implement this slot in case there are any widgets or properties which need
    // to be updated after certain operations, to reflect the inner state correctly.
    // At the moment this is used for smoothing options in the freehand brush, but
    // this will likely be expanded.
    virtual void updateSettingsViews();

Q_SIGNALS:
    void isActiveChanged();

protected:
    // conversion methods are also needed by the paint information builder
    friend class KisToolPaintingInformationBuilder;

    /// Convert from native (postscript points) to image pixel
    /// coordinates.
    QPointF convertToPixelCoord(KoPointerEvent *e);
    QPointF convertToPixelCoord(const QPointF& pt);

protected:
    QPointF widgetCenterInWidgetPixels();
    QPointF convertDocumentToWidget(const QPointF& pt);

    /// Convert from native (postscript points) to integer image pixel
    /// coordinates. This truncates the floating point components and
    /// should be used in preference to QPointF::toPoint(), which rounds,
    /// to ensure the cursor acts on the pixel it is visually over.
    QPoint convertToIntPixelCoord(KoPointerEvent *e);

    QRectF convertToPt(const QRectF &rect);

    QPointF viewToPixel(const QPointF &viewCoord) const;
    /// Convert an integer pixel coordinate into a view coordinate.
    /// The view coordinate is at the centre of the pixel.
    QPointF pixelToView(const QPoint &pixelCoord) const;

    /// Convert a floating point pixel coordinate into a view coordinate.
    QPointF pixelToView(const QPointF &pixelCoord) const;

    /// Convert a pixel rectangle into a view rectangle.
    QRectF pixelToView(const QRectF &pixelRect) const;

    /// Convert a pixel path into a view path
    QPainterPath pixelToView(const QPainterPath &pixelPath) const;

    /// Convert a pixel polygon into a view path
    QPolygonF pixelToView(const QPolygonF &pixelPolygon) const;

    /// Update the canvas for the given rectangle in image pixel coordinates.
    void updateCanvasPixelRect(const QRectF &pixelRect);

    /// Update the canvas for the given rectangle in view coordinates.
    void updateCanvasViewRect(const QRectF &viewRect);

    virtual QWidget* createOptionWidget();

protected:
    KisImageWSP image() const;
    QCursor cursor() const;

    /// Call this to set the document modified
    void notifyModified() const;

    KisImageWSP currentImage();
    KoPattern* currentPattern();
    KoAbstractGradient *currentGradient();
    KisNodeSP currentNode();
    KoColor currentFgColor();
    KoColor currentBgColor();
    KisPaintOpPresetSP currentPaintOpPreset();
    KisFilterConfiguration *currentGenerator();

    virtual void setupPaintAction(KisRecordedPaintAction* action);

    /// paint the path which is in view coordinates, default paint mode is XOR_MODE, BW_MODE is also possible
    /// never apply transformations to the painter, they would be useless, if drawing in OpenGL mode. The coordinates in the path should be in view coordinates.
    void paintToolOutline(QPainter * painter, const QPainterPath &path);

    /// Sets the systemLocked for the current node, this will not deactivate the tool buttons
    void setCurrentNodeLocked(bool locked);

    /// Checks checks if the current node is editable
    bool nodeEditable();

    /// Checks checks if the selection is editable, only applies to local selection as global selection is always editable
    bool selectionEditable();

    /// Override the cursor appropriately if current node is not editable
    bool overrideCursorIfNotEditable();

protected:
    enum ToolMode {
        HOVER_MODE,
        PAINT_MODE,
        SECONDARY_PAINT_MODE,
        MIRROR_AXIS_SETUP_MODE,
        GESTURE_MODE,
        PAN_MODE,
        OTHER // not used now
    };

    virtual void setMode(ToolMode mode);
    virtual ToolMode mode() const;

protected Q_SLOTS:
    /**
     * Called whenever the configuration settings change.
     */
    virtual void resetCursorStyle();

    /**
     * Called when the user requested undo while the stroke is
     * active. If you tool supports undo of the part of its actions,
     * override this method and do the needed work there.
     *
     * NOTE: Default implementation forwards this request to
     *       requestStrokeCancellation() method, so that the stroke
     *       would be cancelled.
     */
    virtual void requestUndoDuringStroke();

    /**
     * Called when the user requested the cancellation of the current
     * stroke. If you tool supports cancelling, override this method
     * and do the needed work there
     */
    virtual void requestStrokeCancellation();

    /**
     * Called when the image decided that the stroke should better be
     * ended. If you tool supports long strokes, override this method
     * and do the needed work there
     */
    virtual void requestStrokeEnd();

private Q_SLOTS:
    void slotToggleFgBg();
    void slotResetFgBg();

private:
    ToolMode m_mode;
    bool m_isActive;

    struct Private;
    Private* const d;
};



#endif // KIS_TOOL_H_

