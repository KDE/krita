/*
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_BRUSH_H_
#define KIS_TOOL_BRUSH_H_

#include "kis_tool_freehand.h"
#include "kis_tool_factory.h"

class QTimer;
class KisPoint;
class QPainter;
class QRect;
class QCheckBox;
class QGridLayout;

class KisToolBrush : public KisToolFreehand {
    Q_OBJECT
    typedef KisToolFreehand super;

public:
    KisToolBrush();
    virtual ~KisToolBrush();
    virtual void update(KisCanvasSubject *subject);
    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual quint32 priority() { return 0; }
    QWidget* createOptionWidget(QWidget* parent);

protected:

    virtual void initPaint(KisEvent *e);
    virtual void endPaint();
    virtual void move(KisMoveEvent *e);
    virtual void leave(QEvent *e);

private slots:

    void timeoutPaint();
    void slotSetPaintingMode( int mode );

private:

    qint32 m_rate;
    QTimer * m_timer;
    QGridLayout* m_optionLayout;
    QCheckBox * m_chkDirect;
};

class KisToolBrushFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolBrushFactory() : super() {};
    virtual ~KisToolBrushFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolBrush();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("brush", i18n("Brush Tool")); }
};


#endif // KIS_TOOL_BRUSH_H_

