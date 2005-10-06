/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_TOOL_NON_PAINT_H_
#define KIS_TOOL_NON_PAINT_H_

#include <qcursor.h>
#include <qcolor.h>
#include <qwidget.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_tool.h"
#include <koffice_export.h>

class QEvent;
class QKeyEvent;
class QPaintEvent;
class QRect;
class KDialog;
class KisCanvasSubject;


class KRITACORE_EXPORT KisToolNonPaint : public KisTool {

    Q_OBJECT
    typedef KisTool super;

public:
    KisToolNonPaint();
    virtual ~KisToolNonPaint();

// CanvasObserver
public:
    virtual void update(KisCanvasSubject *subject);

// KisTool
public:
    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);
    virtual void clear();
    virtual void clear(const QRect& rc);

    virtual void enter(QEvent *e);
    virtual void leave(QEvent *e);
    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);
    virtual void doubleClick(KisDoubleClickEvent *e);
    virtual void keyPress(QKeyEvent *e);
    virtual void keyRelease(QKeyEvent *e);

    virtual QCursor cursor();
    virtual void setCursor(const QCursor& cursor);
    virtual QWidget* createOptionWidget(QWidget* parent) ;
    virtual QWidget* optionWidget();

    virtual enumToolType toolType() { return TOOL_CANVAS; }

public slots:
    virtual void activate();

protected:
    void notifyModified() const;

protected:
    KisCanvasSubject *m_subject;

private:
    QCursor m_cursor;
    QWidget m_optWidget;
};

#endif // KIS_TOOL_NON_PAINT_H_

