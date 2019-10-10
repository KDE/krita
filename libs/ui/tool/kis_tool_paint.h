/*
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_PAINT_H_
#define KIS_TOOL_PAINT_H_

#include "kis_tool.h"

#include <QGridLayout>
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

    bool pickColor(const QPointF &documentPixel, AlternateAction action);

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
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void deactivate() override;

private Q_SLOTS:

    void slotPopupQuickHelp();

    void increaseBrushSize();
    void decreaseBrushSize();

    void activatePickColorDelayed();

    void slotColorPickingFinished(KoColor color);

protected:
    quint8 m_opacity;
    bool m_paintOutline;
    QPointF m_outlineDocPoint;
    QPainterPath m_currentOutline;
    QRectF m_oldOutlineRect;

    bool m_showColorPreview;
    QRectF m_oldColorPreviewRect;
    QRectF m_oldColorPreviewUpdateRect;
    QColor m_colorPreviewCurrentColor;
    bool m_colorPreviewShowComparePlate;
    QColor m_colorPreviewBaseColor;

private:
    QPainterPath tryFixBrushOutline(const QPainterPath &originalOutline);
    void setOpacity(qreal opacity);

    void activatePickColor(AlternateAction action);
    void deactivatePickColor(AlternateAction action);
    void pickColorWasOverridden();

    int colorPreviewResourceId(AlternateAction action);
    QRectF colorPreviewDocRect(const QPointF &outlineDocPoint);

    bool isPickingAction(AlternateAction action);

    struct PickingJob {
        PickingJob() {}
        PickingJob(QPointF _documentPixel,
                   AlternateAction _action)
            : documentPixel(_documentPixel),
              action(_action) {}

        QPointF documentPixel;
        AlternateAction action;
    };
    void addPickerJob(const PickingJob &pickingJob);

private:

    bool m_specialHoverModifier;
    QGridLayout *m_optionsWidgetLayout;

    bool m_supportOutline;

    /**
     * Used as a switch for pickColor
     */

    // used to skip some of the tablet events and don't update the colour that often
    QTimer m_colorPickerDelayTimer;
    AlternateAction delayedAction;

    bool m_isOutlineEnabled;
    std::vector<int> m_standardBrushSizes;

    KisStrokeId m_pickerStrokeId;
    int m_pickingResource;
    typedef KisSignalCompressorWithParam<PickingJob> PickingCompressor;
    QScopedPointer<PickingCompressor> m_colorPickingCompressor;

    qreal m_localOpacity {1.0};
    qreal m_oldOpacity {1.0};

Q_SIGNALS:
    void sigPaintingFinished();
};

#endif // KIS_TOOL_PAINT_H_

