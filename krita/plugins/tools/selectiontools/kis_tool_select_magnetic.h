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
#include <QTransform>

#include <KoToolFactoryBase.h>
#include <KoCreatePathTool.h>
#include "flake/kis_node_shape.h"
#include "kis_tool_select_base.h"
//#include "kis_tool_select_magnetic_option_widget.h"


class QWidget;
class KoCanvasBase;
class KisSelectionOptions;
class KisRandomConstAccessor;
class KoColorTransformation;
class KoColorSpace;

namespace Ui {
    class KisToolSelectMagneticOptionWidget;
}

#include <Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN

typedef Eigen::Matrix3d FilterMatrix;


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
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) {
        m_localTool.activate(toolActivation, shapes);
    }
    virtual void deactivate() {m_localTool.deactivate();}

    int radius() const;
    int threshold() const;
    int searchStartPoint() const;
    int colorLimitation() const;
    bool limitToCurrentLayer() const;

private:
    virtual QWidget* createOptionWidget();
    Ui::KisToolSelectMagneticOptionWidget *m_magneticOptions;

    class LocalTool : public KoCreatePathTool {
        friend class KisToolSelectPath;
    public:
        LocalTool(KoCanvasBase * canvas, KisToolSelectMagnetic* selectingTool);
        ~LocalTool();
        void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
        void deactivate();

        void mouseReleaseEvent(KoPointerEvent *event);
    protected:
        void paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter);
        void paintOutline(QPainter* painter, const QPainterPath &path, qreal zoom);
        virtual void addPathShape(KoPathShape* pathShape);
        void computeOutline(const QPainterPath &pixelPath);
        void computeEdge(const QVector2D &startPoint, const QVector2D &direction, KisRandomConstAccessorSP pixelAccessor);
        FilterMatrix getMatrixForPoint(const QVector2D &point, KisRandomConstAccessorSP pixelAccessor) const;
        FilterMatrix getHorizontalFilterMatrix() const;
        FilterMatrix getVerticalFilterMatrix() const;

        /// this function removes unneeded points in m_detectedBorder, so that it is faster to paint
        void cleanDetectedBorder();

    private:
        KisToolSelectMagnetic* const m_selectingTool;
        KoShapeStroke* m_borderBackup;
        QPolygonF m_outline;
        const KoColorSpace* m_colorSpace;
        KoColorTransformation* m_colorTransformation;
        QPolygon m_detectedBorder;
        QPolygon m_tmpDetectedBorder;
        int m_accuracy;

        //debugging
        QPolygonF m_debugPolyline;
        QPolygon m_debugScannedPoints;
    };
    LocalTool m_localTool;
};

class KisToolSelectMagneticFactory : public KoToolFactoryBase
{

public:
    KisToolSelectMagneticFactory(const QStringList&)
            : KoToolFactoryBase("KisToolSelectMagnetic") {
        setToolTip(i18n("Magnetic selection tool"));
        setToolType(TOOL_TYPE_SELECTED);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
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

