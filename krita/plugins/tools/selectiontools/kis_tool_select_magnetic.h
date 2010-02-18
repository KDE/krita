/*
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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
#ifndef KIS_TOOL_SELECT_MAGNETIC_H_
#define KIS_TOOL_SELECT_MAGNETIC_H_

#include <QPainter>
#include <QPolygonF>

#include <KoToolFactoryBase.h>
#include <KoCreatePathTool.h>
#include "flake/kis_node_shape.h"
#include "kis_tool_select_base.h"

class QWidget;
class KoCanvasBase;
class KisSelectionOptions;
class KisRandomConstAccessor;
class KoColorTransformation;
class KoColorSpace;

/**
 * Tool to select colors by pointing at a color on the image.
 */

class KisToolSelectMagnetic : public KisToolSelectBase
{
    Q_OBJECT
public:
    KisToolSelectMagnetic(KoCanvasBase * canvas);
    virtual ~KisToolSelectMagnetic();

    virtual void paint(QPainter& gc, const KoViewConverter &converter) {m_localTool.paint(gc, converter);}
    virtual void mousePressEvent(KoPointerEvent *e) {m_localTool.mousePressEvent(e);}
    virtual void mouseMoveEvent(KoPointerEvent *e) {m_localTool.mouseMoveEvent(e);}
    virtual void mouseReleaseEvent(KoPointerEvent *e) {m_localTool.mouseReleaseEvent(e);}
    virtual void mouseDoubleClickEvent(KoPointerEvent *e) {m_localTool.mouseDoubleClickEvent(e);}
    virtual void activate(bool temporary) {m_localTool.activate(temporary);}
    virtual void deactivate() {m_localTool.deactivate();}

public slots:
    virtual void slotSetDistance(int);
private:
    virtual QWidget* createOptionWidget();
    int m_distance;

    class LocalTool : public KoCreatePathTool {
        friend class KisToolSelectPath;
    public:
        LocalTool(KoCanvasBase * canvas, KisToolSelectMagnetic* selectingTool);
        ~LocalTool();
        void activate(bool temporary);
        void deactivate();
    protected:
        void paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter);
        void paintOutline(QPainter* painter, const QPainterPath &path, qreal zoom);
        virtual void addPathShape(KoPathShape* pathShape);
        void computeOutline(const QPainterPath &pixelPath);
        void computeEdge(const QVector2D &startPoint, const QVector2D &direction);
    private:
        KisToolSelectMagnetic* const m_selectingTool;
        KoLineBorder* m_borderBackup;
        QPolygonF m_outline;
        KisRandomConstAccessor* m_randomAccessor;
        const KoColorSpace* m_colorSpace;
        KoColorTransformation* m_colorTransformation;
        QPolygon m_detectedBorder;

        //debuging
        QPolygonF m_debugPolyline;
        QPolygon m_debugScannedPoints;
    };
    LocalTool m_localTool;
};

class KisToolSelectMagneticFactory : public KoToolFactoryBase
{

public:
    KisToolSelectMagneticFactory(QObject *parent, const QStringList&)
            : KoToolFactoryBase(parent, "KisToolSelectMagnetic") {
        setToolTip(i18n("Magnetic selection tool"));
        setToolType(TOOL_TYPE_SELECTED);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("tool_magneticoutline_selection");
        setShortcut(KShortcut(Qt::CTRL + Qt::Key_E));
        setPriority(57);
    }

    virtual ~KisToolSelectMagneticFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectMagnetic(canvas);
    }

};


#endif // KIS_TOOL_SELECT_MAGNETIC_H_

