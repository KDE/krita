/*
 *  kis_tool_line.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@comuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_TOOL_GRADIENT_H_
#define KIS_TOOL_GRADIENT_H_

#include "kis_tool_paint.h"

#include "kis_global.h"
#include "kis_types.h"
#include "kis_gradient_painter.h"
#include "kis_tool_factory.h"

class KIntNumInput;
class KDoubleNumInput;

class KisCmbComposite;
class KisPainter;

class QLabel;
class QPoint;
class QWidget;
class QCheckBox;

class KisToolGradient : public KisToolPaint {

    Q_OBJECT
    typedef KisToolPaint super;

public:
    KisToolGradient();
    virtual ~KisToolGradient();

    virtual void setup(KActionCollection *collection);
        virtual enumToolType toolType() { return TOOL_FILL; }

    virtual void update(KisCanvasSubject *subject);

    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);

    virtual void paint(KisCanvasPainter& gc);
    virtual void paint(KisCanvasPainter& gc, const QRect& rc);

    QWidget* createOptionWidget(QWidget* parent);

public slots:
    void slotSetShape(int);
    void slotSetRepeat(int);
    void slotSetReverse(bool);
    void slotSetAntiAliasThreshold(double);

private:
    void paintLine();
    void paintLine(KisCanvasPainter& gc);

    KisPoint straightLine(KisPoint point);

    bool m_dragging;

    KisPoint m_startPos;
    KisPoint m_endPos;

    KisCanvasSubject *m_subject;

    KisGradientPainter::enumGradientShape m_shape;
    KisGradientPainter::enumGradientRepeat m_repeat;

    bool m_reverse;
    double m_antiAliasThreshold;

    QLabel *m_lbShape;
    QLabel *m_lbRepeat;
    QCheckBox *m_ckReverse;
    QComboBox *m_cmbShape;
    QComboBox *m_cmbRepeat;
    QLabel *m_lbAntiAliasThreshold;
    KDoubleNumInput *m_slAntiAliasThreshold;
};

class KisToolGradientFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolGradientFactory() : super() {};
    virtual ~KisToolGradientFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolGradient();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("gradient", i18n("Gradient Tool")); }
};



#endif //KIS_TOOL_GRADIENT_H_

