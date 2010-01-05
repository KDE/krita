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

#include <KoToolFactory.h>

#include <kis_tool_paint.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_gradient_painter.h>
#include <flake/kis_node_shape.h>

#include <opengl/kis_opengl.h>

class KisOpenGLGradientProgram;

class KDoubleNumInput;


class QLabel;
class QPoint;
class QWidget;
class QCheckBox;
class KComboBox;

class KisToolGradient : public KisToolPaint
{

    Q_OBJECT

public:
    KisToolGradient(KoCanvasBase * canvas);
    virtual ~KisToolGradient();

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    QWidget* createOptionWidget();

public slots:
    void slotSetShape(int);
    void slotSetRepeat(int);
    void slotSetReverse(bool);
    void slotSetAntiAliasThreshold(double);

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
    void slotSetPreviewOpacity(qreal value, bool final);
    void slotConfigChanged();
#endif

private slots:

    void areaDone(const QRect & rc) {
        currentNode()->setDirty(rc); // Starts computing the projection for the area we've done.

    }

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
    KComboBox *m_cmbShape;
    KComboBox *m_cmbRepeat;
    QLabel *m_lbAntiAliasThreshold;
    KDoubleNumInput *m_slAntiAliasThreshold;

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
    KisOpenGLGradientProgram *m_gradientProgram;
    int m_previewOpacityPercent;
    QLabel *m_lbPreviewOpacity;
    KoSliderCombo *m_slPreviewOpacity;
#endif
};

class KisToolGradientFactory : public KoToolFactory
{

public:
    KisToolGradientFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KritaFill/KisToolGradient") {
        setToolTip(i18n("Draw a gradient."));
        setToolType(TOOL_TYPE_FILL);
        setIcon("krita_tool_gradient");
        setShortcut(KShortcut(Qt::Key_G));
        setPriority(15);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolGradientFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolGradient(canvas);
    }

};

#endif //KIS_TOOL_GRADIENT_H_

