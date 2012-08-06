/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_TOOL_PAN_H_
#define KIS_TOOL_PAN_H_

#include <KoPanTool.h>
#include <KoToolFactoryBase.h>
#include <kis_tool.h>
#include <KoIcon.h>

class KoCanvasBase;
class KisCanvas2;

class KisToolPan : public KisTool
{

    Q_OBJECT

public:
    KisToolPan(KoCanvasBase * canvas);
    virtual ~KisToolPan();


public:

    bool wantsAutoScroll() const;
    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

private:
    void adjustCursor();
    bool isInCheckerArea(QPointF pt);
    qreal calculateAngle(QPointF oldPoint, QPointF newPoint);
    KisCanvas2* kritaCanvas() const;

private:
    static const qreal m_checkerRadius;

    QCursor m_defaultCursor;
    QPointF m_lastPosition;
    bool m_rotationMode;
};


class KisToolPanFactory : public KoToolFactoryBase
{

public:
    KisToolPanFactory(const QStringList&)
            : KoToolFactoryBase(KoPanTool_ID) {
        setToolTip(i18n("Move and rotate your canvas"));
        setToolType(navigationToolType());
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID_ALWAYS_ACTIVE);
        setPriority(5);
        setIconName(koIconNameCStr("krita_tool_pan"));
        //setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_V ) );
    }

    virtual ~KisToolPanFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolPan(canvas);
    }

};

#endif // KIS_TOOL_PAN_H_

