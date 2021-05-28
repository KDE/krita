/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_H_
#define KIS_TOOL_H_

#include <QCursor>

#include <KoColor.h>
#include <KoToolBase.h>
#include <KoID.h>
#include <KoCanvasResourceProvider.h>
#include <KoPattern.h>
#include <KoAbstractGradient.h>

#include <kis_types.h>

#ifdef __GNUC__
#define WARN_WRONG_MODE(_mode) warnKrita << "Unexpected tool event has come to" << __func__ << "while being mode" << _mode << "!"
#else
#define WARN_WRONG_MODE(_mode) warnKrita << "Unexpected tool event has come while being mode" << _mode << "!"
#endif

#define CHECK_MODE_SANITY_OR_RETURN(_mode) if (mode() != _mode) { WARN_WRONG_MODE(mode()); return; }

class KoCanvasBase;
class KisFilterConfiguration;
class QPainter;
class QPainterPath;
class QPolygonF;

//activation id for Krita tools, Krita tools are always active and handle locked and invisible layers by themself
static const QString KRITA_TOOL_ACTIVATION_ID = "flake/always";

#include <kritaui_export.h>
class  KRITAUI_EXPORT KisTool : public KoToolBase
{
    Q_OBJECT

    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    enum { FLAG_USES_CUSTOM_PRESET=0x01, FLAG_USES_CUSTOM_COMPOSITEOP=0x02, FLAG_USES_CUSTOM_SIZE=0x04 };

    KisTool(KoCanvasBase * canvas, const QCursor & cursor);
    ~KisTool() override;

    virtual int flags() const { return 0; }

    void deleteSelection() override;
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
     * tool is supposed to revert all the preparations it has done in
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
        AlternateChangeSizeSnap,
        AlternateSampleFgNode,
        AlternateSampleBgNode,
        AlternateSampleFgImage,
        AlternateSampleBgImage,
        AlternateSecondary,
        AlternateThird,
        AlternateFourth,
        AlternateFifth,
        Alternate_NONE = 10000
    };

    // Technically users are allowed to configure this, but nobody ever would do that.
    // So these can basically be thought of as aliases to ctrl+click, etc.
    enum AlternateAction {
        ChangeSize = AlternateChangeSize, // Default: Shift+Left click
        ChangeSizeSnap = AlternateChangeSizeSnap, // Default: Shift+Z+Left click
        SampleFgNode = AlternateSampleFgNode, // Default: Ctrl+Alt+Left click
        SampleBgNode = AlternateSampleBgNode, // Default: Ctrl+Alt+Right click
        SampleFgImage = AlternateSampleFgImage, // Default: Ctrl+Left click
        SampleBgImage = AlternateSampleBgImage, // Default: Ctrl+Right click
        Secondary = AlternateSecondary,
        Third = AlternateThird,
        Fourth = AlternateFourth,
        Fifth = AlternateFifth,
        NONE = 10000
    };

    enum NodePaintAbility {
        VECTOR,
        CLONE,
        PAINT,
        UNPAINTABLE,
        MYPAINTBRUSH_UNPAINTABLE
    };
    Q_ENUMS(NodePaintAbility)

    static AlternateAction actionToAlternateAction(ToolAction action);

    virtual void activateAlternateAction(AlternateAction action);
    virtual void deactivateAlternateAction(AlternateAction action);

    virtual void beginAlternateAction(KoPointerEvent *event, AlternateAction action);
    virtual void continueAlternateAction(KoPointerEvent *event, AlternateAction action);
    virtual void endAlternateAction(KoPointerEvent *event, AlternateAction action);
    virtual void beginAlternateDoubleClickAction(KoPointerEvent *event, AlternateAction action);
    virtual bool alternateActionSupportsHiResEvents(AlternateAction action) const;

    void mousePressEvent(KoPointerEvent *event) override;
    void mouseDoubleClickEvent(KoPointerEvent *event) override;
    void mouseTripleClickEvent(KoPointerEvent *event) override;
    void mouseReleaseEvent(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;

    bool isActive() const;

    KisTool::NodePaintAbility nodePaintAbility();

    /**
     * @brief newActivationWithExternalSource
     * Makes sure that the tool is active and starts a new stroke, which will
     * be able to access the pixels from the specified external source.
     *
     * This is currently implemented by the Transform tool to paste an image
     * into the current layer and transform it.
     */
    virtual void newActivationWithExternalSource(KisPaintDeviceSP externalSource);

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void canvasResourceChanged(int key, const QVariant & res) override;
    // Implement this slot in case there are any widgets or properties which need
    // to be updated after certain operations, to reflect the inner state correctly.
    // At the moment this is used for smoothing options in the freehand brush, but
    // this will likely be expanded.
    virtual void updateSettingsViews();

Q_SIGNALS:
    void isActiveChanged(bool isActivated);

protected:
    // conversion methods are also needed by the paint information builder
    friend class KisToolPaintingInformationBuilder;

    /// Convert from native (postscript points) to image pixel
    /// coordinates.
    QPointF convertToPixelCoord(KoPointerEvent *e);
    QPointF convertToPixelCoord(const QPointF& pt);

    QPointF convertToPixelCoordAndAlignOnWidget(const QPointF& pt);

    QPointF convertToPixelCoordAndSnap(KoPointerEvent *e, const QPointF &offset = QPointF(), bool useModifiers = true);
    QPointF convertToPixelCoordAndSnap(const QPointF& pt, const QPointF &offset = QPointF());

protected:
    QPointF widgetCenterInWidgetPixels();
    QPointF convertDocumentToWidget(const QPointF& pt);

    /// Convert from native (postscript points) to integer image pixel
    /// coordinates. This rounds down (not truncate) the pixel coordinates and
    /// should be used in preference to QPointF::toPoint(), which rounds,
    /// to ensure the cursor acts on the pixel it is visually over.
    QPoint convertToImagePixelCoordFloored(KoPointerEvent *e);

    QRectF convertToPt(const QRectF &rect);
    qreal convertToPt(qreal value);

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

    QWidget* createOptionWidget() override;

    /**
     * To determine whether this tool will change its behavior when
     * modifier keys are pressed
     */
    virtual bool listeningToModifiers();
    /**
     * Request that this tool no longer listen to modifier keys
     * (Responding to the request is optional)
     */
    virtual void listenToModifiers(bool listen);

protected:
    KisImageWSP image() const;
    QCursor cursor() const;

    KisImageWSP currentImage();
    KoPatternSP currentPattern();
    KoAbstractGradientSP currentGradient();
    KisNodeSP currentNode() const;
    KisNodeList selectedNodes() const;
    KoColor currentFgColor();
    KoColor currentBgColor();
    KisPaintOpPresetSP currentPaintOpPreset();
    KisFilterConfigurationSP currentGenerator();

    /// paint the path which is in view coordinates, default paint mode is XOR_MODE, BW_MODE is also possible
    /// never apply transformations to the painter, they would be useless, if drawing in OpenGL mode. The coordinates in the path should be in view coordinates.
    void paintToolOutline(QPainter * painter, const QPainterPath &path);

    /// Checks checks if the current node is editable
    bool nodeEditable();

    /// Checks checks if the selection is editable, only applies to local selection as global selection is always editable
    bool selectionEditable();

    /// Override the cursor appropriately if current node is not editable
    bool overrideCursorIfNotEditable();

    bool blockUntilOperationsFinished();
    void blockUntilOperationsFinishedForced();

protected:
    enum ToolMode: int {
        HOVER_MODE,
        PAINT_MODE,
        SECONDARY_PAINT_MODE,
        MIRROR_AXIS_SETUP_MODE,
        GESTURE_MODE,
        PAN_MODE,
        OTHER, // tool-specific modes, like multibrush's symmetry axis setup
        OTHER_1
    };

    virtual void setMode(ToolMode mode);
    virtual ToolMode mode() const;
    void setCursor(const QCursor &cursor);

protected Q_SLOTS:
    /**
     * Called whenever the configuration settings change.
     */
    virtual void resetCursorStyle();

private:
    struct Private;
    Private* const d;
};



#endif // KIS_TOOL_H_
