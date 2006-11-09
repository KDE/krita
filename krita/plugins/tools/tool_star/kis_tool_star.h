/*
 *  kis_tool_star.h - part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael Thaler@physik.tu-muenchen.de>
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

#ifndef KIS_TOOL_STAR_H_
#define KIS_TOOL_STAR_H_

#include "kis_tool_shape.h"
#include "ui_wdg_tool_star.h"

class KisCanvas;
class KisDoc;
class KisPainter;
class KisView;
class KisRect;

class WdgToolStar : public QWidget, public Ui::WdgToolStar
{
    Q_OBJECT

    public:
        WdgToolStar(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class KisToolStar : public KisToolShape {

    typedef KisToolShape super;
    Q_OBJECT

public:
    KisToolStar();
    virtual ~KisToolStar();

    virtual QWidget* createOptionWidget();

    //
    // KisToolPaint interface
    //

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual void buttonPress(KoPointerEvent *event);
    virtual void move(KoPointerEvent *event);
    virtual void buttonRelease(KoPointerEvent *event);

protected:
    virtual void draw(const KoPoint& start, const KoPoint& stop);
    //virtual void draw(KisPainter *gc, const QRect& rc);

protected:
    int m_lineThickness;

    KoPoint m_dragStart;
    KoPoint m_dragEnd;
    QRect m_final_lines;

    bool m_dragging;
    KisImageSP m_currentImage;
private:
    vKoPoint starCoordinates(int N, double mx, double my, double x, double y);
    qint32 m_innerOuterRatio;
    qint32 m_vertices;
    WdgToolStar* m_optWidget;
};


#include "KoToolFactory.h"

class KisToolStarFactory : public KoToolFactory {

public:
    KisToolStarFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolStar", i18n( "Star" ))
        {
            setToolTip(i18n("Draw a star with the current brush"));
            setToolType(TOOL_TYPE_SHAPE);
            setPriority(0);
            setIcon("tool_star");
        }

    virtual ~KisToolStarFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolStar(canvas);
    }

};


#endif //__KIS_TOOL_STAR_H__
