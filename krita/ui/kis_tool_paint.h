/*
 *  Copyright (c) 2003 Boudewijn Rempt
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


#include <KoTool.h>

#include <krita_export.h>

#include "kis_tool.h"
#include "KoCompositeOp.h"

class QCheckBox;
class QEvent;
class QKeyEvent;
class QComboBox;
class QPaintEvent;
class QRect;
class QGridLayout;
class QLabel;

class KDialog;

class KoCanvasBase;

class KisCmbComposite;
class KisIntSpinbox;

enum enumBrushMode {
    PAINT,
    PAINT_STYLUS,
    ERASE,
    ERASE_STYLUS,
    HOVER
};

class KRITAUI_EXPORT KisToolPaint
    : public KoTool {

    Q_OBJECT

public:
    KisToolPaint(KoCanvasBase * canvas);
    virtual ~KisToolPaint();

public slots:

    virtual void activate(bool temporary = false);
    virtual void deactivate();


public:

    virtual void paint(QPainter& gc, KoViewConverter &converter);

    virtual void mousePressEvent( KoPointerEvent *event );
    virtual void mouseDoubleClickEvent( KoPointerEvent *event );
    virtual void mouseMoveEvent( KoPointerEvent *event );
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void wheelEvent ( KoPointerEvent * event );


    virtual QCursor cursor();
    virtual void setCursor(const QCursor& cursor);
    virtual void addOptionWidgetOption(QWidget *control, QWidget *label = 0);

    void slotSetOpacity(int opacityPerCent);
    void slotSetCompositeMode(const KoCompositeOp* compositeOp);
    void slotPopupQuickHelp();

protected:
    void notifyModified() const;

    // Add the tool-specific layout to the default option widget's layout.
    void addOptionWidgetLayout(QLayout *layout);

    virtual QWidget* createOptionWidget(QWidget* parent);

private:
    // XXX: Call this when the layer changes (this used to be called in KisCanvasObserver::update)
    void updateCompositeOpComboBox();

protected:

    QRect m_dirtyRect;
    quint8 m_opacity;
    const KoCompositeOp * m_compositeOp;
    bool m_paintOutline;

private:
    QCursor m_cursor;

    QGridLayout *m_optionWidgetLayout;

    QLabel *m_lbOpacity;
    KisIntSpinbox *m_slOpacity;
    QLabel *m_lbComposite;
    KisCmbComposite *m_cmbComposite;
};

#endif // KIS_TOOL_PAINT_H_

