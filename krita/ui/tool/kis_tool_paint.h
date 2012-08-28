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

#include <kis_pattern.h>

#include "kis_tool.h"
#include "KoCompositeOp.h"
#include <QCheckBox>

class QEvent;
class QKeyEvent;
class QPaintEvent;
class QGridLayout;
class QLabel;
class QPoint;


class KoCanvasBase;

class KisSliderSpinBox;

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

    virtual void resourceChanged(int key, const QVariant & v);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent* event);


    /** If the paint tool support outline like brushes, set to true.
    *   If not (e.g. gradient tool), set to false. Default is false.
    */
    void setSupportOutline(bool supportOutline) {
        m_supportOutline = supportOutline;
    }


protected:
    bool specialHoverModeActive() const;


    /// Add the tool-specific layout to the default option widget layout.
    void addOptionWidgetLayout(QLayout *layout);

    /// Add a widget and a label to the current option widget layout.
    virtual void addOptionWidgetOption(QWidget *control, QWidget *label = 0);

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

public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();

private slots:

    void slotPopupQuickHelp();
    void slotSetOpacity(qreal opacity);

    void makeColorLighter();
    void makeColorDarker();

    void increaseOpacity();
    void decreaseOpacity();

protected slots:
    virtual void resetCursorStyle();
    virtual void updateTabletPressureSamples();


protected:
    quint8 m_opacity;
    bool m_paintOutline;
    QVector<qreal> m_pressureSamples;

private:
    void pickColor(const QPointF &documentPixel, bool fromCurrentNode,
                   bool toForegroundColor);

    void transformColor(int step);
    void stepAlpha(float step);

private:

    bool m_specialHoverModifier;
    QGridLayout *m_optionWidgetLayout;

    bool m_supportOutline;

    /**
     * Used as a switch for pickColor
     */
    bool m_toForegroundColor;
    // used to skip some of the tablet events and don't update the colour that often
    QTimer m_colorPickerDelayTimer;

signals:
    void sigFavoritePaletteCalled(const QPoint&);
    void sigPaintingFinished();
};

#endif // KIS_TOOL_PAINT_H_

