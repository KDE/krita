/*
 *  Copyright (c) 2003-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_TOOL_MULTIHAND_H_
#define KIS_TOOL_MULTIHAND_H_

#include "kis_types.h"
#include "kis_tool_paint.h"
#include "kis_paint_information.h"

#include "krita_export.h"

// OpenGL
#include <opengl/kis_opengl.h>

class QStackedWidget;
class KisTransaction;
class KisPainter;
class KAction;

class KoPointerEvent;
class KoCanvasBase;

class KisPainter;

class QThreadPool;
class QComboBox;
class FreehandPaintJob;
class KisRecordedPathPaintAction;
class FreehandPaintJobExecutor;

#include "kis_paintop_settings.h"
#include <kis_cursor.h>
#include <KoToolFactoryBase.h>
#include <QPushButton>


class KisToolMultihand : public KisToolPaint
{
    Q_OBJECT
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    KisToolMultihand(KoCanvasBase * canvas, const QCursor & cursor, const QString & transactionText);
    virtual ~KisToolMultihand();
    virtual int flags() const;
    virtual void setDirty(const QVector<QRect> &rects);
    virtual void setDirty(const QRegion &region);

protected:
    void gesture(const QPointF &offsetInDocPixels,
                 const QPointF &initialDocPoint);

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual bool wantsAutoScroll() const;
    virtual void deactivate();


    /// Paint a single brush footprint on the current layer
    virtual void paintAt(const KisPaintInformation &pi, KisPainter * painter);

    /// Paint a line between the specified positions on the current layer
    virtual void paintLine(const KisPaintInformation &pi1,
                           const KisPaintInformation &pi2,
                           KisPainter * painter);

    virtual void paintBezierCurve(const KisPaintInformation &pi1,
                                  const QPointF &control1,
                                  const QPointF &control2,
                                  const KisPaintInformation &pi2,
                                  KisPainter * painter
                                 );

    virtual void initPaint(KoPointerEvent *e);
    virtual void endPaint();
    virtual void paint(QPainter& gc, const KoViewConverter &converter);

protected slots:

    void setSmooth(bool smooth);
    void setAssistant(bool assistant);
    virtual QWidget * createOptionWidget();

private:
    /**
     * adjust a coordinates according to a KisPaintingAssitant, if available.
     */
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    void queuePaintJob(FreehandPaintJob* job, FreehandPaintJob* previousJob);
    void showOutlineTemporary();

    void updateOutlineRect();
    QPainterPath getOutlinePath(const QPointF &documentPos,
                                KisPaintOpSettings::OutlineMode outlineMode);

    void initTransformations();
    void updateCanvas();
    void finishAxisSetup();

    QGridLayout *m_optionLayout;
    QCheckBox *m_chkSmooth;
    QCheckBox *m_chkAssistant;
    KisSliderSpinBox *m_sliderMagnetism;
    KisSliderSpinBox *m_sliderSmoothness;
    QComboBox * m_transformModes;
    KisSliderSpinBox *m_brushCounter;
    QStackedWidget *m_modeCustomOption;
    QCheckBox *m_mirrorVerticallyChCkBox;
    QCheckBox *m_mirrorHorizontallyChCkBox;
    KisSliderSpinBox *m_translateRadiusSlider;
    QPushButton *m_axisPointBtn;

private slots:
    void increaseBrushSize();
    void decreaseBrushSize();
    void hideOutline();
    void activateAxisPointModeSetup();

    void slotSetSmoothness(int smoothness);
    void slotSetMagnetism(int magnetism);
    void slotSetBrushCount(int count);
    void slotSetCurrentTransformMode(int qcomboboxIndex);
    void slotSetMirrorVertically(bool mirror);
    void slotSetMirrorHorizontally(bool mirror);
    void slotSetTranslateRadius(int radius);
    void timeoutPaint();

protected:

    KisPaintInformation m_previousPaintInformation;
    QPointF m_previousTangent;
    double m_dragDist;
    QPointF m_strokeBegin;

    bool m_paintIncremental;

    QString m_transactionText;

    bool m_smooth;
    double m_smoothness;
    bool m_assistant;
    double m_magnetism;

private:
#if defined(HAVE_OPENGL)
    qreal m_xTilt;
    qreal m_yTilt;

    qreal m_prevxTilt;
    qreal m_prevyTilt;

    GLuint m_displayList;
    QString m_brushModelName;
#endif

    int m_brushesCount;
    QVector<KisPainter *> m_painters;
    QVector<QTransform> m_brushTransforms;
    QPointF m_translatedBrush;
    KisTransaction * m_transaction;
    bool m_mirrorVertically;
    bool m_mirrorHorizontally;
    int m_translateRadius;
    QPointF m_axisPoint;
    bool m_dragAxis;

    enum enumTransforModes { SYMETRY, MIRROR, TRANSLATE, CUSTOM };

    enumTransforModes m_currentTransformMode;


    QPointF m_outlineDocPoint;
    QTimer m_outlineTimer;
    QRectF m_oldOutlineRect;
    bool m_explicitShowOutline;

    QTimer *m_airbrushTimer;
    qint32 m_rate;
    bool m_isAirbrushing;

    QRegion m_incrementalDirtyRegion;
    QList<FreehandPaintJob*> m_paintJobs;
    KisRecordedPathPaintAction* m_pathPaintAction;
    QThreadPool* m_executor;

    QTime m_strokeTimeMeasure;

    KAction* m_increaseBrushSize;
    KAction* m_decreaseBrushSize;

    bool m_hasPaintAtLeastOnce; ///< this indicates whether mouseReleaseEvent should call paintAt or not
};

class KisToolMultiBrushFactory : public KoToolFactoryBase
{

public:
    KisToolMultiBrushFactory(const QStringList&)
            : KoToolFactoryBase("KritaShape/KisToolMultiBrush") {

        setToolTip(i18n("Paint with multibrushes"));

        // Temporarily
        setToolType(TOOL_TYPE_SHAPE);
        setIcon("krita_tool_freehand");
        setShortcut(KShortcut(Qt::Key_Q));
        setPriority(11);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolMultiBrushFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolMultihand(canvas,KisCursor::load("tool_freehand_cursor.png", 5, 5), i18n("MultiBrush"));
    }

};



#endif // KIS_TOOL_FREEHAND_H_

