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
class KDialog;
class KisCanvasSubject;
class QLabel;
class KisCmbComposite;
class KisIntSpinbox;

enum enumBrushMode {
    PAINT,
    PAINT_STYLUS,
    ERASE,
    ERASE_STYLUS,
    HOVER
};

class KRITAUI_EXPORT KisToolPaint : public KisTool {

    Q_OBJECT
    typedef KisTool super;

public:
    KisToolPaint(const QString& UIName);
    virtual ~KisToolPaint();

public:
    virtual void update(KisCanvasSubject *subject);

    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);

    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);
    virtual void doubleClick(KisDoubleClickEvent *e);
    virtual void keyPress(QKeyEvent *e);
    virtual void keyRelease(QKeyEvent *e);

    virtual QCursor cursor();
    virtual void setCursor(const QCursor& cursor);
    virtual QWidget* createOptionWidget(QWidget* parent);
    virtual QWidget* optionWidget();
    virtual void addOptionWidgetOption(QWidget *control, QWidget *label = 0);

public slots:
    virtual void activate();
    virtual void deactivate();

    void slotSetOpacity(int opacityPerCent);
    void slotSetCompositeMode(const KoCompositeOp* compositeOp);
    void slotPopupQuickHelp();

protected:
    void notifyModified() const;

    // Add the tool-specific layout to the default option widget's layout.
    void addOptionWidgetLayout(QLayout *layout);

private:
    void updateCompositeOpComboBox();

protected:
    KisCanvasSubject *m_subject;
    QRect m_dirtyRect;
    quint8 m_opacity;
    const KoCompositeOp * m_compositeOp;
    bool m_paintOutline;

private:
    QString m_UIName;

    QCursor m_cursor;

    QWidget *m_optionWidget;
    QGridLayout *m_optionWidgetLayout;

    QLabel *m_lbOpacity;
    KisIntSpinbox *m_slOpacity;
    QLabel *m_lbComposite;
    KisCmbComposite *m_cmbComposite;
};

#endif // KIS_TOOL_PAINT_H_

