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

#include <kshortcut.h>

#include "KoToolFactory.h"

#include "kis_tool_paint.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_gradient_painter.h"

#include <kis_layer_shape.h>

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
    KisToolGradient(KoCanvasBase * canvas);
    virtual ~KisToolGradient();

 //   virtual void setup(KActionCollection *collection);
 //       virtual enumToolType toolType() { return TOOL_FILL; }

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint( QPainter &painter, KoViewConverter &converter );

    QWidget* createOptionWidget();

public slots:
    void slotSetShape(int);
    void slotSetRepeat(int);
    void slotSetReverse(bool);
    void slotSetAntiAliasThreshold(double);

private:
    void paintLine(QPainter& gc);

    QPointF straightLine(QPointF point);

    bool m_dragging;

    QPointF m_startPos;
    QPointF m_endPos;

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

class KisToolGradientFactory : public KoToolFactory {

public:
    KisToolGradientFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KritaFill/KisToolGradient", i18n( "Gradient" ))
        {
            setToolTip( i18n( "Draw a gradient." ) );
            //setToolType( TOOL_TYPE_FILL );
            setToolType( dynamicToolType() );
            setIcon( "tool_gradient" );
            setShortcut( KShortcut( Qt::Key_G ) );
            setPriority( 0 );
            setActivationShapeId( KIS_LAYER_SHAPE_ID );
        }

    virtual ~KisToolGradientFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolGradient(canvas);
    }

};

#endif //KIS_TOOL_GRADIENT_H_

