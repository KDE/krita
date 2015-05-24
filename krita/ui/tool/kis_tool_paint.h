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

#include <vector>

#include <QCursor>
#include <QLayout>
#include <QLabel>
#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QVariant>
#include <QTimer>

#include <KoCanvasResourceManager.h>
#include <KoToolBase.h>
#include <KoAbstractGradient.h>

#include <krita_export.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paintop_settings.h>

#include <KoPattern.h>

#include "kis_tool.h"
#include <QCheckBox>

class QEvent;
class QKeyEvent;
class QPaintEvent;
class QGridLayout;
class QLabel;
class QPoint;
class KoCompositeOp;


class KoCanvasBase;


// wacom
const static int LEVEL_OF_PRESSURE_RESOLUTION = 1024;

class KRITAUI_EXPORT KisToolPaint : public KisTool
{

    Q_OBJECT

public:
    KisToolPaint(KoCanvasBase * canvas, const QCursor & cursor);
    virtual ~KisToolPaint();
    virtual int flags() const;

protected:

    void setMode(ToolMode mode);

    virtual void canvasResourceChanged(int key, const QVariant & v);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

    virtual void activatePrimaryAction();
    virtual void deactivatePrimaryAction();

    virtual void activateAlternateAction(AlternateAction action);
    virtual void deactivateAlternateAction(AlternateAction action);

    virtual void beginAlternateAction(KoPointerEvent *event, AlternateAction action);
    virtual void continueAlternateAction(KoPointerEvent *event, AlternateAction action);
    virtual void endAlternateAction(KoPointerEvent *event, AlternateAction action);

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);

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

    virtual QWidget * createOptionWidget();

    /**
     * Quick help is a short help text about the way the tool functions.
     */
    virtual QString quickHelp() const {
        return QString();
    }

    virtual void setupPaintAction(KisRecordedPaintAction* action);

    qreal pressureToCurve(qreal pressure){
        return m_pressureSamples.at( qRound(pressure * LEVEL_OF_PRESSURE_RESOLUTION) );
    }

    enum NodePaintAbility {
        NONE,
        PAINT,
        VECTOR
    };

    /// Checks if and how the tool can paint on the current node
    NodePaintAbility nodePaintAbility();

    const KoCompositeOp* compositeOp();

public Q_SLOTS:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();

private Q_SLOTS:

    void slotPopupQuickHelp();

    void increaseBrushSize();
    void decreaseBrushSize();

    void activatePickColorDelayed();

protected Q_SLOTS:
    virtual void updateTabletPressureSamples();


protected:
    quint8 m_opacity;
    bool m_paintOutline;
    QVector<qreal> m_pressureSamples;
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

Q_SIGNALS:
    void sigPaintingFinished();
};

#endif // KIS_TOOL_PAINT_H_

