/*
 *  kis_tool_transform.h - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2010 Marc Pegon
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

#ifndef KIS_TOOL_TRANSFORM_H_
#define KIS_TOOL_TRANSFORM_H_

#include <complex>

#include <QPoint>
#include <QPointF>
#include <QVector3D>
#include <QButtonGroup>

#include <KoInteractionTool.h>
#include <KoToolFactoryBase.h>

#include <kis_shape_selection.h>
#include <kis_undo_adapter.h>
#include <kis_types.h>
#include <flake/kis_node_shape.h>
#include <kis_tool.h>

#include "ui_wdg_tool_transform.h"
#include "tool_transform_args.h"

//#define PERSPECTIVE_DISABLED

class KoID;
class KisFilterStrategy;

class WdgToolTransform : public QWidget, public Ui::WdgToolTransform
{
    Q_OBJECT

public:
    WdgToolTransform(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

typedef enum {NONE, XCOORD, YCOORD} DICHO_DROP;
/**
 * Transform tool
 *
 */
class KisToolTransform : public KisTool, KisCommandHistoryListener
{

    Q_OBJECT

public:
    KisToolTransform(KoCanvasBase * canvas);
    virtual ~KisToolTransform();

    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    void paint(QPainter& gc, const KoViewConverter &converter);
    //recalc the outline & current QImages
    void recalcOutline();
    //update the boundrect of the current transformed pixels
    void updateCurrentOutline();
    //recalcs the outline and update the corresponding areas of the canvas (union of the outline boundrect before & after recalc)
    void updateOutlineChanged();
    double dichotomyScaleY(QVector3D v1, QVector3D v2, DICHO_DROP flag, double desired, double b, double precision, double maxIterations1, double maxIterations2);
    double dichotomyScaleX(QVector3D v1, QVector3D v2, DICHO_DROP flag, double desired, double b, double precision, double maxIterations1, double maxIterations2);
    QPolygonF aux_dichotomyScaleXY(QPointF v1, QPointF v2, QPointF v3, QPointF v4, QVector3D v1Proj, double scaleX, double scaleY);
    QPointF dichotomyScaleXY(QPointF v1, QPointF v2, QPointF v3, QPointF v4, QVector3D v1Proj, QVector3D v3Desired, int signX, int signY, double precision, double maxIterations1, double maxIterations2);
    //sets the value of the spinboxes to current args
    void refreshSpinBoxes();
    void setButtonBoxDisabled(bool disabled);

public:

    void notifyCommandAdded(const QUndoCommand *);
    void notifyCommandExecuted(const QUndoCommand *);

public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();
    void setRotCenter(int id);
    void setScaleX(double scaleX);
    void setScaleY(double scaleY);
    void setShearX(double shearX);
    void setShearY(double shearY);
    void setAX(double aX);
    void setAY(double aY);
    void setAZ(double aZ);
    void setTranslateX(double translateX);
    void setTranslateY(double translateY);
    void buttonBoxClicked(QAbstractButton *button);
    void keepAspectRatioChanged(bool keep);
    void editingFinished();

private:

    void transform(); //only commits the current transformation to the undo stack
    void applyTransform(); //applies the current transformation to the original paint device

    QVector3D rotX(double x, double y, double z) {
        return QVector3D(x, m_cosaX * y - m_sinaX * z, m_cosaX * z + m_sinaX * y);
    }
    QVector3D invrotX(double x, double y, double z) {
        return QVector3D(x, m_cosaX * y + m_sinaX * z, m_cosaX * z - m_sinaX * y);
    }
    QVector3D rotY(double x, double y, double z) {
        return QVector3D(m_cosaY * x + m_sinaY * z, y, - m_sinaY * x + m_cosaY * z);
    }
    QVector3D invrotY(double x, double y, double z) {
        return QVector3D(m_cosaY * x - m_sinaY * z, y, + m_sinaY * x + m_cosaY * z);
    }
    QPointF perspective(double x, double y, double z) {
        if (!m_currentArgs.aX() && !m_currentArgs.aY())
            return QPointF(x, y);

        QVector3D t = QVector3D(x, y, z) - m_cameraPos;
        if (t.z() == 0.0)
            return QPointF(0,0);

        return QPointF((t.x() - m_eyePos.x()) * m_eyePos.z() / t.z(), (t.y() - m_eyePos.y()) * m_eyePos.z() / t.z());
    }
    //the perspective is only invertible if you five the plan into which the returned point should be
    QVector3D invperspective(double x, double y, QVector3D plan) {
        //the following is the solution of the system :
        //x = (xinv - cx)*ez / (zinv - cz)
        //y = (yinv - cy)*ez / (zinv - cz)
        //a*xinv + b*yinv + c*zinv = 0
        //where :
        //    (cx, cy, cz) = (m_cameraPos.x() + m_eyePos.x(), m_cameraPos.y() + m_eyePos.y(), m_cameraPos.z())
        //    ez = m_eyePos
        //    (a, b, c) = plan
        if (!m_currentArgs.aX() && !m_currentArgs.aY())
            return QVector3D(x, y, 0);

        double a = plan.x();
        double b = plan.y();
        double c = plan.z();
        double denom = a*x + b*y + c*m_eyePos.z();

        if (!denom)
            return QVector3D(0, 0, 0);

        double cx = (m_cameraPos.x() - m_eyePos.x());
        double cy = (m_cameraPos.y() - m_eyePos.y());
        double cz = m_cameraPos.z();
        double ez = m_eyePos.z();
        double acx = a * cx;
        double bcy = b * cy;
        double ccz = c * cz;
        double xinv, yinv, zinv;
        xinv = (- (ccz + bcy) * x + cx * (b*y + c*ez)) / denom;
        yinv = (cy * (a*x + c*ez) - (ccz + acx) * y) / denom;
        zinv = ((a*x + b*y) * cz - (acx + bcy) * ez) / denom;

        return QVector3D(xinv, yinv, zinv);
    }

    QVector3D rotZ(double x, double y, double z) {
        return QVector3D(m_cosaZ*x - m_sinaZ*y, m_sinaZ*x + m_cosaZ*y, z);
    }
    QVector3D invrotZ(double x, double y, double z) {
        return QVector3D(m_cosaZ*x + m_sinaZ*y, -m_sinaZ*x + m_cosaZ*y, z);
    }
    QVector3D shear(double x, double y, double z) {
        double x2 = x + y * m_currentArgs.shearX();
        return QVector3D(x2, y + x2 * m_currentArgs.shearY(), z);
    }
    QVector3D invshear(double x, double y, double z) {
        y -= x * m_currentArgs.shearY();
        x -= y * m_currentArgs.shearX();

        return QVector3D(x, y, z);
    }
    QVector3D scale(double x, double y, double z) {
        return QVector3D(x * m_currentArgs.scaleX(), y * m_currentArgs.scaleY(), z);
    }
    QVector3D invscale(double x, double y, double z) {
        return QVector3D(x / m_currentArgs.scaleX(), y / m_currentArgs.scaleY(), z);
    }

    //add 2 scale factors to take view resolution into account (when making QImage previews)
    QVector3D prev_transformVector(double x, double y, double z, double resX, double resY) {
        QVector3D t = scale(x, y ,z);
        t = QVector3D(t.x() * resX, t.y() * resY, t.z());
        t = shear(t.x(), t.y(), t.z());
        t = rotZ(t.x(), t.y(), t.z());
        t = rotY(t.x(), t.y(), t.z());
        t = rotX(t.x(), t.y(), t.z());

        return t;
    }
    QVector3D transformVector(double x, double y, double z) {
        QVector3D t = scale(x, y ,z);
        t = shear(t.x(), t.y(), t.z());
        t = rotZ(t.x(), t.y(), t.z());
        t = rotY(t.x(), t.y(), t.z());
        t = rotX(t.x(), t.y(), t.z());

        return t;
    }
    QVector3D invTransformVector(double x, double y, double z) {
        QVector3D t = invrotX(x, y, z);
        t = invrotY(t.x(), t.y(), t.z());
        t = invrotZ(t.x(), t.y(), t.z());
        t = invshear(t.x(), t.y(), t.z());
        t = invscale(t.x(), t.y(), t.z());

        return t;
    }
    QVector3D transformVector(QVector3D v) {
        return transformVector(v.x(), v.y(), v.z());
    }
    QVector3D prev_transformVector(QVector3D v, double resX, double resY) {
        return prev_transformVector(v.x(), v.y(), v.z(), resX, resY);
    }
    QVector3D invTransformVector(QVector3D v) {
        return invTransformVector(v.x(), v.y(), v.z());
    }

    int det(const QPointF & v, const QPointF & w);
    double distsq(const QPointF & v, const QPointF & w); //square of the euclidian distance
    int octant(double x, double y); //the octant of the director given by vector (x,y)
    //sets the cursor according the mouse position (doesn't take shearing into account yet)
    void setFunctionalCursor();
    void setTransformFunction(QPointF mousePos, Qt::KeyboardModifiers modifiers);
    //just sets default values for current args, temporary values..
    void initTransform();
    //saves the original selection, paintDevice, Images previous. set transformation to default using initTransform
    void initHandles();
    //stores m_currentArgs into args
    void storeArgs(ToolTransformArgs &args);
    //sets m_currentArgs to args
    void restoreArgs(ToolTransformArgs args);

private slots:

    void slotSetFilter(const KoID &);

private:
    enum function {ROTATE = 0, MOVE, RIGHTSCALE, TOPRIGHTSCALE, TOPSCALE, TOPLEFTSCALE,
                   LEFTSCALE, BOTTOMLEFTSCALE, BOTTOMSCALE, BOTTOMRIGHTSCALE, 
                   BOTTOMSHEAR, RIGHTSHEAR, TOPSHEAR, LEFTSHEAR,
                   MOVECENTER, PERSPECTIVE
                  };
    QPointF m_handleDir[9];

    QCursor m_sizeCursors[8]; //cursors for the 8 directions
    function m_function; //current transformation function

    ToolTransformArgs m_currentArgs;
    ToolTransformArgs m_clickArgs;

    int m_handleRadius;
    int m_rotationCenterRadius;
    int m_maxRadius;

    //informations on the original selection (before any transformation)
    double m_originalWidth2, m_originalHeight2; //'2' for half
    QPoint m_originalTopLeft;  //in image coords
    QPoint m_originalBottomRight;  //in image coords
    QPointF m_originalCenter; //original center of the selection

    //informations on the bounding rect of the selection just after the LAST transformation
    //thus they are initially equal to original points
    //(used to know dirty rects more precisely)
    QPoint m_previousTopLeft;  //in image coords
    QPoint m_previousBottomRight;  //in image coords

    //center used for rotation (calculated from rotationCenterOffset (in currentArgs))
    QVector3D m_rotationCenter;
    QPointF m_clickRotationCenterProj; //the rotation center projection at click
    QPointF m_rotationCenterProj;

    bool m_selecting; // true <=> selection has been clicked
    bool m_actuallyMoveWhileSelected; // true <=> selection has been moved while clicked
    bool m_imageTooBig;

    //informations on the current selection
    QVector3D m_topLeft;  //in image coords
    QVector3D m_topRight;  //in image coords
    QVector3D m_bottomLeft;  //in image coords
    QVector3D m_bottomRight;  //in image coords
    QVector3D m_middleLeft;
    QVector3D m_middleRight;
    QVector3D m_middleTop;
    QVector3D m_middleBottom;

    QPointF m_topLeftProj; //perspective projection of m_topLeft
    QPointF m_topRightProj;
    QPointF m_bottomLeftProj;
    QPointF m_bottomRightProj;
    QPointF m_middleLeftProj;
    QPointF m_middleRightProj;
    QPointF m_middleTopProj;
    QPointF m_middleBottomProj;

    QPointF m_clickTopLeftProj;
    QPointF m_clickTopRightProj;
    QPointF m_clickBottomLeftProj;
    QPointF m_clickBottomRightProj;
    QPointF m_clickMiddleLeftProj;
    QPointF m_clickMiddleRightProj;
    QPointF m_clickMiddleTopProj;
    QPointF m_clickMiddleBottomProj;

    QVector3D m_cameraPos, m_eyePos;
    QVector3D m_currentPlan, m_clickPlan; //vector (a, b, c) represents the vect plan a*x + b*y + c*z = 0

    //current scale factors
    //wOutModifiers don't take shift modifier into account
    double m_scaleX_wOutModifier;
    double m_scaleY_wOutModifier;

    QPointF m_prevMousePos;
    QPointF m_clickPoint; //position of the mouse when click occured

    double m_cosaZ; //cos and sin of transformArgs.aZ()
    double m_sinaZ;
    double m_cosaX;
    double m_sinaX;
    double m_cosaY;
    double m_sinaY;
    double m_clickangle; //angle made at click, from the rotationCenter

    bool m_boxValueChanged; //true if a boxValue has been changed directly by the user (not by click + move mouse)

    QImage *m_origImg; //image of the pixels in selection bound rect
    QTransform m_transform; //transformation from the origImg
    QImage m_currImg; //origImg transformed using m_transform
    QImage *m_origSelectionImg; //the original selection with white used as alpha channel
    QImage m_scaledOrigSelectionImg; //the original selection to be drawn, scaled to the view
    QSizeF m_refSize; //used in paint() to check if the view has changed (need to update m_currSelectionImg);

    KisFilterStrategy *m_filter;

    WdgToolTransform *m_optWidget;

    KisPaintDeviceSP m_target;
    //we don't need this origDevice for now
    //but i keep it here because i might use it when adding one of enkithan's suggestion (cut the seleted pixels instead of keeping them darkened)
    KisPaintDeviceSP m_origDevice;
    KisSelectionSP m_origSelection; //contains the original selection
    //KisShapeSelection *m_previousShapeSelection;
    KoCanvasBase *m_canvas;

    QButtonGroup *m_rotCenterButtons;
};

class KisToolTransformFactory : public KoToolFactoryBase
{


public:

    KisToolTransformFactory(QObject *parent, const QStringList&)
            : KoToolFactoryBase(parent, "KisToolTransform") {
        setToolTip(i18n("Transform a layer or a selection"));
        setToolType(TOOL_TYPE_TRANSFORM);
        setIcon("krita_tool_transform");
        setPriority(11);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    virtual ~KisToolTransformFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolTransform(canvas);
    }

};



#endif // KIS_TOOL_TRANSFORM_H_

