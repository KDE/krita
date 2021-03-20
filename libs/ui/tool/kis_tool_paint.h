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
#include "kis_signal_compressor_with_param.h"
#include <brushengine/kis_paintop_settings.h>
#include <resources/KoPattern.h>

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

    virtual QPainterPath getOutlinePath(const QPointF &documentPos,
                                        const KoPointerEvent *event,
                                        KisPaintOpSettings::OutlineMode outlineMode);

protected:
    bool isOutlineEnabled() const;
    void setOutlineEnabled(bool enabled);

    bool sampleColor(const QPointF &documentPixel, AlternateAction action);

    /// Add the tool-specific layout to the default option widget layout.
    void addOptionWidgetLayout(QLayout *layout);

    /// Add a widget and a label to the current option widget layout.
    virtual void addOptionWidgetOption(QWidget *control, QWidget *label = 0);

    void showControl(QWidget *control, bool value);
    void enableControl(QWidget *control, bool value);

    QWidget * createOptionWidget() override;

    /**
     * Quick help is a short help text about the way the tool functions.
     */
    virtual QString quickHelp() const {
        return QString();
    }

    const KoCompositeOp* compositeOp();

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

private Q_SLOTS:

    void slotPopupQuickHelp();

    void increaseBrushSize();
    void decreaseBrushSize();

    void activateSampleColorDelayed();

    void slotColorSamplingFinished(KoColor color);

protected:
    quint8 m_opacity;
    bool m_paintOutline {false};
    QPointF m_outlineDocPoint;
    QPainterPath m_currentOutline;
    QRectF m_oldOutlineRect;

    bool m_showColorPreview;
    QRectF m_oldColorPreviewRect;
    QRectF m_oldColorPreviewUpdateRect;
    QColor m_colorPreviewCurrentColor;
    bool m_colorPreviewShowComparePlate;
    QRectF m_oldColorPreviewBaseColorRect;
    QColor m_colorPreviewBaseColor;

private:
    QPainterPath tryFixBrushOutline(const QPainterPath &originalOutline);
    void setOpacity(qreal opacity);

    void activateSampleColor(AlternateAction action);
    void deactivateSampleColor(AlternateAction action);
    void sampleColorWasOverridden();

    int colorPreviewResourceId(AlternateAction action);
    std::pair<QRectF, QRectF> colorPreviewDocRect(const QPointF &outlineDocPoint);

    bool isSamplingAction(AlternateAction action);

    struct SamplingJob {
        SamplingJob() {}
        SamplingJob(QPointF _documentPixel,
                   AlternateAction _action)
            : documentPixel(_documentPixel),
              action(_action) {}

        QPointF documentPixel;
        AlternateAction action;
    };
    void addSamplerJob(const SamplingJob &samplingJob);

private:

    bool m_specialHoverModifier;
    QGridLayout *m_optionsWidgetLayout;

    bool m_supportOutline;

    /**
     * Used as a switch for sampleColor
     */

    // used to skip some of the tablet events and don't update the colour that often
    QTimer m_colorSamplerDelayTimer;
    AlternateAction delayedAction {AlternateAction::NONE};

    bool m_isOutlineEnabled;
    std::vector<int> m_standardBrushSizes;

    KisStrokeId m_samplerStrokeId;
    int m_samplingResource {0};
    typedef KisSignalCompressorWithParam<SamplingJob> SamplingCompressor;
    QScopedPointer<SamplingCompressor> m_colorSamplingCompressor;

    qreal m_localOpacity {1.0};
    qreal m_oldOpacity {1.0};

Q_SIGNALS:
    void sigPaintingFinished();
};

#endif // KIS_TOOL_PAINT_H_

