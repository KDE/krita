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

#include <KoCanvasResourceProvider.h>
#include <KoTool.h>
#include <KoAbstractGradient.h>

#include <krita_export.h>

#include <kis_types.h>
#include <kis_image.h>

#include <kis_pattern.h>

#include "kis_tool.h"
#include "KoCompositeOp.h"

class QEvent;
class QKeyEvent;
class QPaintEvent;
class QGridLayout;
class QLabel;
class QPoint;


class KoCanvasBase;

class KisCmbComposite;
class KoSliderCombo;

enum enumBrushMode {
    PAINT,
    PAINT_STYLUS,
    ERASE,
    ERASE_STYLUS,
    HOVER,
    EDIT_BRUSH,
    PAN
};

class KRITAUI_EXPORT KisToolPaint
        : public KisTool
{

    Q_OBJECT

public:
    KisToolPaint(KoCanvasBase * canvas, const QCursor & cursor);
    virtual ~KisToolPaint();


public:

    virtual void resourceChanged(int key, const QVariant & v);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

    virtual void mouseReleaseEvent(KoPointerEvent *event);

    /** If the paint tool support outline like brushes, set to true.
    *   If not (e.g. gradient tool), set to false. Default is false.
    */
    void setSupportOutline(bool supportOutline) {
        m_supportOutline = supportOutline;
    }


protected:

    /// Add the tool-specific layout to the default option widget layout.
    void addOptionWidgetLayout(QLayout *layout);

    /// Add a widget and a label to the current option widget layout.
    virtual void addOptionWidgetOption(QWidget *control, QWidget *label = 0);

    virtual QWidget * createOptionWidget();

    /** Quick help is a short help text about the way the tool functions.
    * Deprecated: this method may move to KoToolFactory.
    */
    virtual KDE_DEPRECATED QString quickHelp() const {
        return QString();
    }


public slots:
    virtual void activate(bool temporary = false);

private slots:

    void updateCompositeOpComboBox();
    void slotPopupQuickHelp();
    void slotSetOpacity(qreal opacityPerCent, bool final);
    void slotSetCompositeMode(const QString& compositeOp);

protected slots:
    virtual void resetCursorStyle();


protected:

    quint8 m_opacity;
    const KoCompositeOp * m_compositeOp;
    bool m_paintOutline;

private:

    QGridLayout *m_optionWidgetLayout;

    QLabel *m_lbOpacity;
    KoSliderCombo *m_slOpacity;
    QLabel *m_lbComposite;
    KisCmbComposite *m_cmbComposite;
    KisNodeSP m_previousNode;

    bool m_supportOutline;

signals:
    void favoritePaletteCalled(const QPoint&);
};

#endif // KIS_TOOL_PAINT_H_

