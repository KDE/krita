/*
 *  SPDX-FileCopyrightText: 2003 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_PAINT_H_
#define KIS_TOOL_PAINT_H_

#include "kis_tool.h"

#include <QGridLayout>
#include <QPainterPath>
#include <QTimer>
#include <QCheckBox>

#include <KoCanvasResourceProvider.h>
#include <KoToolBase.h>
#include <resources/KoAbstractGradient.h>

#include <kritaui_export.h>

#include <kis_types.h>
#include <kis_image.h>
#include <brushengine/kis_paintop_settings.h>
#include <resources/KoPattern.h>
#include <KisOptimizedBrushOutline.h>
#include "KisAsyncColorSamplerHelper.h"

class QGridLayout;
class KoCompositeOp;
class KoCanvasBase;

class KRITAUI_EXPORT KisToolPaint : public KisTool
{

    Q_OBJECT

public:
    KisToolPaint(KoCanvasBase *canvas, const QCursor &cursor);
    ~KisToolPaint() override;
    int flags() const override;

    void mousePressEvent(KoPointerEvent *event) override;
    void mouseReleaseEvent(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;

    KisPopupWidgetInterface* popupWidget() override;

protected:

    void setMode(ToolMode mode) override;

    void canvasResourceChanged(int key, const QVariant &v) override;

    void paint(QPainter &gc, const KoViewConverter &converter) override;

    void activatePrimaryAction() override;
    void deactivatePrimaryAction() override;

    void activateAlternateAction(AlternateAction action) override;
    void deactivateAlternateAction(AlternateAction action) override;

    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void endAlternateAction(KoPointerEvent *event, AlternateAction action) override;

    virtual void requestUpdateOutline(const QPointF &outlineDocPoint, const KoPointerEvent *event);

    /** If the paint tool support outline like brushes, set to true.
    *   If not (e.g. gradient tool), set to false. Default is false.
    */
    void setSupportOutline(bool supportOutline) {
        m_supportOutline = supportOutline;
    }

    virtual KisOptimizedBrushOutline getOutlinePath(const QPointF &documentPos,
                                                    const KoPointerEvent *event,
                                                    KisPaintOpSettings::OutlineMode outlineMode);

protected:
    bool isOutlineEnabled() const;
    void setOutlineEnabled(bool enabled);
    bool isOutlineVisible() const;
    void setOutlineVisible(bool visible);

    bool isEraser() const;

    /// Add the tool-specific layout to the default option widget layout.
    void addOptionWidgetLayout(QLayout *layout);

    /// Add a widget and a label to the current option widget layout.
    virtual void addOptionWidgetOption(QWidget *control, QWidget *label = nullptr);

    void showControl(QWidget *control, bool value);
    void enableControl(QWidget *control, bool value);

    QWidget * createOptionWidget() override;

    /**
     * Quick help is a short help text about the way the tool functions.
     */
    virtual QString quickHelp() const {
        return QString();
    }

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

private Q_SLOTS:

    void slotColorPickerRequestedCursor(const QCursor &cursor);
    void slotColorPickerRequestedCursorReset();
    void slotColorPickerRequestedOutlineUpdate();

    void slotPopupQuickHelp();

    void increaseBrushSize();
    void decreaseBrushSize();
    void showBrushSize();

    void rotateBrushTipClockwise();
    void rotateBrushTipClockwisePrecise();
    void rotateBrushTipCounterClockwise();
    void rotateBrushTipCounterClockwisePrecise();

protected:
    quint8 m_opacity {OPACITY_OPAQUE_U8};
    bool m_paintOutline {false};
    QPointF m_outlineDocPoint;
    KisOptimizedBrushOutline m_currentOutline;
    QRectF m_oldOutlineRect;
    QRectF m_oldColorPreviewUpdateRect;

private:
    KisOptimizedBrushOutline tryFixBrushOutline(const KisOptimizedBrushOutline &originalOutline);
    void setOpacity(qreal opacity);
    bool isSamplingAction(AlternateAction action);
private:

    bool m_specialHoverModifier {false};
    QGridLayout *m_optionsWidgetLayout {nullptr};

    bool m_supportOutline {false};

    /**
     * Used as a switch for sampleColor
     */

    bool m_isOutlineEnabled;
    bool m_isOutlineVisible;
    std::vector<int> m_standardBrushSizes;

    KisPaintOpPresetSP m_oldPreset;
    qreal m_oldOpacity {1.0};
    bool m_oldPresetIsDirty {false};
    int m_oldPresetVersion {-1};
    KisAsyncColorSamplerHelper m_colorSamplerHelper;

    void tryRestoreOpacitySnapshot();

    struct Private;
    QScopedPointer<Private> m_d;

Q_SIGNALS:
    void sigPaintingFinished();
};

#endif // KIS_TOOL_PAINT_H_

