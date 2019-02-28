/*
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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
#include "kis_tool_multihand.h"

#include <QTransform>

#include <QPushButton>
#include <QComboBox>
#include <QFormLayout>
#include <QStackedWidget>
#include <kis_slider_spin_box.h>
#include <QLabel>
#include "kis_canvas2.h"
#include "kis_cursor.h"

#include "kis_tool_multihand_helper.h"


static const int MAXIMUM_BRUSHES = 50;

#include <QtGlobal>
#ifdef Q_OS_WIN
// quoting DRAND48(3) man-page:
// These functions are declared obsolete by  SVID  3,
// which  states  that rand(3) should be used instead.
#define drand48() (static_cast<double>(qrand()) / static_cast<double>(RAND_MAX))
#endif


KisToolMultihand::KisToolMultihand(KoCanvasBase *canvas)
    : KisToolBrush(canvas),
      m_transformMode(SYMMETRY),
      m_angle(0),
      m_handsCount(6),
      m_mirrorVertically(false),
      m_mirrorHorizontally(false),
      m_showAxes(false),
      m_translateRadius(100),
      m_setupAxesFlag(false),
      m_addSubbrushesMode(false)
    , customUI(0)
{


    m_helper =
        new KisToolMultihandHelper(paintingInformationBuilder(),
                                   kundo2_i18n("Multibrush Stroke"));
    resetHelper(m_helper);
    if (image()) {
        m_axesPoint = QPointF(0.5 * image()->width(), 0.5 * image()->height());
    }

}

KisToolMultihand::~KisToolMultihand()
{
}

void KisToolMultihand::beginPrimaryAction(KoPointerEvent *event)
{
    if(m_setupAxesFlag) {
        setMode(KisTool::OTHER);
        m_axesPoint = convertToPixelCoord(event->point);
        requestUpdateOutline(event->point, 0);
        updateCanvas();
    }
    else if (m_addSubbrushesMode){
        QPointF newPoint = convertToPixelCoord(event->point);
        m_subbrOriginalLocations << newPoint;
        requestUpdateOutline(event->point, 0);
        updateCanvas();
    }
    else {
        initTransformations();
        KisToolFreehand::beginPrimaryAction(event);
    }
}

void KisToolMultihand::continuePrimaryAction(KoPointerEvent *event)
{
    if(mode() == KisTool::OTHER) {
        m_axesPoint = convertToPixelCoord(event->point);
        requestUpdateOutline(event->point, 0);
        updateCanvas();
    }
    else {
        requestUpdateOutline(event->point, 0);
        KisToolFreehand::continuePrimaryAction(event);
    }
}

void KisToolMultihand::endPrimaryAction(KoPointerEvent *event)
{
    if(mode() == KisTool::OTHER) {
        setMode(KisTool::HOVER_MODE);
        requestUpdateOutline(event->point, 0);
        finishAxesSetup();
    }
    else {
        KisToolFreehand::endPrimaryAction(event);
    }
}

void KisToolMultihand::beginAlternateAction(KoPointerEvent* event, AlternateAction action)
{
    if (action != ChangeSize || m_transformMode != COPYTRANSLATE || !m_addSubbrushesMode) {
        KisToolBrush::beginAlternateAction(event, action);
        return;
    }
    setMode(KisTool::OTHER_1);
    m_axesPoint = convertToPixelCoord(event->point);
    requestUpdateOutline(event->point, 0);
    updateCanvas();
}

void KisToolMultihand::continueAlternateAction(KoPointerEvent* event, AlternateAction action)
{
    if (action != ChangeSize || m_transformMode != COPYTRANSLATE || !m_addSubbrushesMode) {
        KisToolBrush::continueAlternateAction(event, action);
        return;
    }
    if (mode() == KisTool::OTHER_1) {
        m_axesPoint = convertToPixelCoord(event->point);
        requestUpdateOutline(event->point, 0);
        updateCanvas();
    }
}

void KisToolMultihand::endAlternateAction(KoPointerEvent* event, AlternateAction action)
{
    if (action != ChangeSize || m_transformMode != COPYTRANSLATE || !m_addSubbrushesMode) {
        KisToolBrush::endAlternateAction(event, action);
        return;
    }
    if (mode() == KisTool::OTHER_1) {
        setMode(KisTool::HOVER_MODE);
    }
}

void KisToolMultihand::mouseMoveEvent(KoPointerEvent* event)
{
    if (mode() == HOVER_MODE) {
        m_lastToolPos=convertToPixelCoord(event->point);
    }
    KisToolBrush::mouseMoveEvent(event);
}

void KisToolMultihand::paint(QPainter& gc, const KoViewConverter &converter)
{
    QPainterPath path;

    if (m_showAxes) {
        int axisLength = currentImage()->height() + currentImage()->width();

        // add division guide lines if using multiple brushes
        if ((m_handsCount > 1 && m_transformMode == SYMMETRY) ||
            (m_handsCount > 1 && m_transformMode == SNOWFLAKE) ) {

            qreal axesAngle = 360.0 / float(m_handsCount);
            float currentAngle = 0.0;
            float startingInsetLength = 20; // don't start each line at the origin so we can see better when all points converge

            // draw lines radiating from the origin
            for( int i=0; i < m_handsCount; i++) {

                currentAngle = i*axesAngle;

                // convert angles to radians since cos and sin need that
                currentAngle = currentAngle * 0.017453 + m_angle; // m_angle is current rotation set on UI

                QPoint startingSpot = QPoint(m_axesPoint.x()+ (sin(currentAngle)*startingInsetLength), m_axesPoint.y()- (cos(currentAngle))*startingInsetLength );
                path.moveTo(startingSpot.x(), startingSpot.y());
                QPointF symmetryLinePoint(m_axesPoint.x()+ (sin(currentAngle)*axisLength), m_axesPoint.y()- (cos(currentAngle))*axisLength );
                path.lineTo(symmetryLinePoint);
            }

        }
        else if(m_transformMode == MIRROR) {

            if (m_mirrorHorizontally) {
                path.moveTo(m_axesPoint.x()-axisLength*cos(m_angle+M_PI_2), m_axesPoint.y()-axisLength*sin(m_angle+M_PI_2));
                path.lineTo(m_axesPoint.x()+axisLength*cos(m_angle+M_PI_2), m_axesPoint.y()+axisLength*sin(m_angle+M_PI_2));
            }

            if(m_mirrorVertically) {
                path.moveTo(m_axesPoint.x()-axisLength*cos(m_angle), m_axesPoint.y()-axisLength*sin(m_angle));
                path.lineTo(m_axesPoint.x()+axisLength*cos(m_angle), m_axesPoint.y()+axisLength*sin(m_angle));
            }
        }
        else if (m_transformMode == COPYTRANSLATE) {

            int ellipsePreviewSize = 10;
            // draw ellipse at origin to emphasize this is a drawing point
            path.addEllipse(m_axesPoint.x()-(ellipsePreviewSize),
                            m_axesPoint.y()-(ellipsePreviewSize),
                            ellipsePreviewSize*2,
                            ellipsePreviewSize*2);

            for (QPointF dPos : m_subbrOriginalLocations) {
                path.addEllipse(dPos, ellipsePreviewSize, ellipsePreviewSize);  // Show subbrush reference locations while in add mode
            }

            // draw the horiz/vertical line for axis  origin
            path.moveTo(m_axesPoint.x()-axisLength*cos(m_angle), m_axesPoint.y()-axisLength*sin(m_angle));
            path.lineTo(m_axesPoint.x()+axisLength*cos(m_angle), m_axesPoint.y()+axisLength*sin(m_angle));
            path.moveTo(m_axesPoint.x()-axisLength*cos(m_angle+M_PI_2), m_axesPoint.y()-axisLength*sin(m_angle+M_PI_2));
            path.lineTo(m_axesPoint.x()+axisLength*cos(m_angle+M_PI_2), m_axesPoint.y()+axisLength*sin(m_angle+M_PI_2));

        }
        else {

            // draw the horiz/vertical line for axis  origin
            path.moveTo(m_axesPoint.x()-axisLength*cos(m_angle), m_axesPoint.y()-axisLength*sin(m_angle));
            path.lineTo(m_axesPoint.x()+axisLength*cos(m_angle), m_axesPoint.y()+axisLength*sin(m_angle));
            path.moveTo(m_axesPoint.x()-axisLength*cos(m_angle+M_PI_2), m_axesPoint.y()-axisLength*sin(m_angle+M_PI_2));
            path.lineTo(m_axesPoint.x()+axisLength*cos(m_angle+M_PI_2), m_axesPoint.y()+axisLength*sin(m_angle+M_PI_2));
        }

    } else {

        // not showing axis
        if (m_transformMode == COPYTRANSLATE) {

            for (QPointF dPos : m_subbrOriginalLocations) {
                // Show subbrush reference locations while in add mode
                if (m_addSubbrushesMode) {
                    path.addEllipse(dPos, 10, 10);
                }
            }
        }
    }

    KisToolFreehand::paint(gc, converter);

    // origin point preview line/s
    gc.save();
    QPen outlinePen;
    outlinePen.setColor(QColor(100,100,100,150));
    outlinePen.setStyle(Qt::PenStyle::SolidLine);
    gc.setPen(outlinePen);
    paintToolOutline(&gc, pixelToView(path));
    gc.restore();


    // fill in a dot for the origin if showing axis
    if (m_showAxes) {
        // draw a dot at the origin point to help with precisly moving
        QPainterPath dotPath;
        int dotRadius = 4;
        dotPath.moveTo(m_axesPoint.x(), m_axesPoint.y());
        dotPath.addEllipse(m_axesPoint.x()- dotRadius*0.25, m_axesPoint.y()- dotRadius*0.25, dotRadius, dotRadius); // last 2 parameters are dot's size

        QBrush fillBrush;
        fillBrush.setColor(QColor(255, 255, 255, 255));
        fillBrush.setStyle(Qt::SolidPattern);
        gc.fillPath(pixelToView(dotPath), fillBrush);


        // add slight offset circle for contrast to help show it on
        dotPath = QPainterPath(); // resets path
        dotPath.addEllipse(m_axesPoint.x() - dotRadius*0.75, m_axesPoint.y()- dotRadius*0.75, dotRadius, dotRadius); // last 2 parameters are dot's size
        fillBrush.setColor(QColor(120, 120, 120, 255));
        gc.fillPath(pixelToView(dotPath), fillBrush);
    }

}

void KisToolMultihand::initTransformations()
{
    QVector<QTransform> transformations;
    QTransform m;

    if(m_transformMode == SYMMETRY) {
        qreal angle = 0;
        qreal angleStep = (2 * M_PI) / m_handsCount;

        for(int i = 0; i < m_handsCount; i++) {
            m.translate(m_axesPoint.x(), m_axesPoint.y());
            m.rotateRadians(angle);
            m.translate(-m_axesPoint.x(), -m_axesPoint.y());

            transformations << m;
            m.reset();
            angle += angleStep;
        }
    }
    else if(m_transformMode == MIRROR) {
        transformations << m;

        if (m_mirrorHorizontally) {
            m.translate(m_axesPoint.x(),m_axesPoint.y());
            m.rotateRadians(m_angle);
            m.scale(-1,1);
            m.rotateRadians(-m_angle);
            m.translate(-m_axesPoint.x(), -m_axesPoint.y());
            transformations << m;
            m.reset();
        }

        if (m_mirrorVertically) {
            m.translate(m_axesPoint.x(),m_axesPoint.y());
            m.rotateRadians(m_angle);
            m.scale(1,-1);
            m.rotateRadians(-m_angle);
            m.translate(-m_axesPoint.x(), -m_axesPoint.y());
            transformations << m;
            m.reset();
        }

        if (m_mirrorVertically && m_mirrorHorizontally){
            m.translate(m_axesPoint.x(),m_axesPoint.y());
            m.rotateRadians(m_angle);
            m.scale(-1,-1);
            m.rotateRadians(-m_angle);
            m.translate(-m_axesPoint.x(), -m_axesPoint.y());
            transformations << m;
            m.reset();
        }

    }
    else if(m_transformMode == SNOWFLAKE) {
        qreal angle = 0;
        qreal angleStep = (2 * M_PI) / m_handsCount/4;

        for(int i = 0; i < m_handsCount*4; i++) {
           if ((i%2)==1) {

               m.translate(m_axesPoint.x(), m_axesPoint.y());
               m.rotateRadians(m_angle-angleStep);
               m.rotateRadians(angle);
               m.scale(-1,1);
               m.rotateRadians(-m_angle+angleStep);
               m.translate(-m_axesPoint.x(), -m_axesPoint.y());

               transformations << m;
               m.reset();
               angle += angleStep*2;
           } else {
               m.translate(m_axesPoint.x(), m_axesPoint.y());
               m.rotateRadians(m_angle-angleStep);
               m.rotateRadians(angle);
               m.rotateRadians(-m_angle+angleStep);
               m.translate(-m_axesPoint.x(), -m_axesPoint.y());

               transformations << m;
               m.reset();
               angle += angleStep*2;
            }
        }
    }
    else if(m_transformMode == TRANSLATE) {
        /**
         * TODO: currently, the seed is the same for all the
         * strokes
         */
        for (int i = 0; i < m_handsCount; i++){
            qreal angle = drand48() * M_PI * 2;
            qreal length = drand48();

            // convert the Polar coordinates to Cartesian coordinates
            qreal nx = (m_translateRadius * cos(angle) * length);
            qreal ny = (m_translateRadius * sin(angle) * length);

            m.translate(m_axesPoint.x(),m_axesPoint.y());
            m.rotateRadians(m_angle);
            m.translate(nx,ny);
            m.rotateRadians(-m_angle);
            m.translate(-m_axesPoint.x(), -m_axesPoint.y());
            transformations << m;
            m.reset();
        }
    } else if (m_transformMode == COPYTRANSLATE) {
        transformations << m;
        for (QPointF dPos : m_subbrOriginalLocations) {
            QPointF resPos = dPos-m_axesPoint; // Calculate the difference between subbrush reference position and "origin" reference
            m.translate(resPos.x(), resPos.y());
            transformations << m;
            m.reset();
        }
    }

    m_helper->setupTransformations(transformations);
}

QWidget* KisToolMultihand::createOptionWidget()
{
    QWidget *widget = KisToolBrush::createOptionWidget();

    customUI = new KisToolMultiHandConfigWidget();

    // brush smoothing option.
    //customUI->layout()->addWidget(widget);
    customUI->smoothingOptionsLayout->addWidget(widget);


    // setup common parameters that all of the modes will see
    connect(customUI->showAxesCheckbox, SIGNAL(toggled(bool)), this, SLOT(slotSetAxesVisible(bool)));
    customUI->showAxesCheckbox->setChecked((bool)m_configGroup.readEntry("showAxes", false));

    connect(image(), SIGNAL(sigSizeChanged(QPointF,QPointF)), this, SLOT(resetAxes()));

    customUI->moveOriginButton->setCheckable(true);
    connect(customUI->moveOriginButton, SIGNAL(clicked(bool)),this, SLOT(activateAxesPointModeSetup()));

    connect(customUI->resetOriginButton, SIGNAL(released()), this, SLOT(resetAxes()));

    customUI->multihandTypeCombobox->addItem(i18n("Symmetry"),int(SYMMETRY));  // axis mode
    customUI->multihandTypeCombobox->addItem(i18n("Mirror"),int(MIRROR));
    customUI->multihandTypeCombobox->addItem(i18n("Translate"),int(TRANSLATE));
    customUI->multihandTypeCombobox->addItem(i18n("Snowflake"),int(SNOWFLAKE));
    customUI->multihandTypeCombobox->addItem(i18n("Copy Translate"),int(COPYTRANSLATE));
    connect(customUI->multihandTypeCombobox,SIGNAL(currentIndexChanged(int)),this, SLOT(slotSetTransformMode(int)));
    customUI->multihandTypeCombobox->setCurrentIndex(m_configGroup.readEntry("transformMode", 0));
    slotSetTransformMode(customUI->multihandTypeCombobox->currentIndex());


    customUI->axisRotationSpinbox->setSuffix(QChar(Qt::Key_degree));   // origin rotation
    customUI->axisRotationSpinbox->setSingleStep(0.5);
    customUI->axisRotationSpinbox->setRange(0.0, 90.0, 1);
    customUI->axisRotationSpinbox->setValue(m_configGroup.readEntry("axesAngle", 0.0));
    connect( customUI->axisRotationSpinbox, SIGNAL(valueChanged(qreal)),this, SLOT(slotSetAxesAngle(qreal)));


    // symmetry mode options
    customUI->brushCountSpinBox->setRange(1, MAXIMUM_BRUSHES);
    connect(customUI->brushCountSpinBox, SIGNAL(valueChanged(int)),this, SLOT(slotSetHandsCount(int)));
    customUI->brushCountSpinBox->setValue(m_configGroup.readEntry("handsCount", 4));

    // mirror mode specific options
    connect(customUI->horizontalCheckbox, SIGNAL(toggled(bool)), this, SLOT(slotSetMirrorHorizontally(bool)));
    customUI->horizontalCheckbox->setChecked((bool)m_configGroup.readEntry("mirrorHorizontally", false));

    connect(customUI->verticalCheckbox, SIGNAL(toggled(bool)), this, SLOT(slotSetMirrorVertically(bool)));
    customUI->verticalCheckbox->setChecked((bool)m_configGroup.readEntry("mirrorVertically", false));

    // translate mode options
    customUI->translationRadiusSpinbox->setRange(0, 200);
    customUI->translationRadiusSpinbox->setSuffix(i18n(" px"));
    customUI->translationRadiusSpinbox->setValue(m_configGroup.readEntry("translateRadius", 0));

    connect(customUI->translationRadiusSpinbox,SIGNAL(valueChanged(int)),this,SLOT(slotSetTranslateRadius(int)));

    // Copy translate mode options and actions
    connect(customUI->addSubbrushButton, &QPushButton::clicked, this, &KisToolMultihand::slotAddSubbrushesMode);
    connect(customUI->removeSubbrushButton, &QPushButton::clicked, this, &KisToolMultihand::slotRemoveAllSubbrushes);

    // snowflake re-uses the existing options, so there is no special parameters for that...


    return static_cast<QWidget*>(customUI); // keeping it in the native class until the end allows us to access the UI components
}

void KisToolMultihand::activateAxesPointModeSetup()
{
    if (customUI->moveOriginButton->isChecked()){
        m_setupAxesFlag = true;
        useCursor(KisCursor::crossCursor());
        updateCanvas();
    } else {
        finishAxesSetup();
    }
}

void KisToolMultihand::resetAxes()
{
    m_axesPoint = QPointF(0.5 * image()->width(), 0.5 * image()->height());
    finishAxesSetup();
}


void KisToolMultihand::finishAxesSetup()
{
    m_setupAxesFlag = false;
    customUI->moveOriginButton->setChecked(false);
    resetCursorStyle();
    updateCanvas();
}

void KisToolMultihand::updateCanvas()
{
    KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);
    kisCanvas->updateCanvas();
}

void KisToolMultihand::slotSetHandsCount(int count)
{
    m_handsCount = count;
    m_configGroup.writeEntry("handsCount", count);
    updateCanvas();
}

void KisToolMultihand::slotSetAxesAngle(qreal angle)
{
    //negative so axes rotates counter clockwise
    m_angle = -angle*M_PI/180;
    updateCanvas();
    m_configGroup.writeEntry("axesAngle", angle);
}

void KisToolMultihand::slotSetTransformMode(int index)
{
    m_transformMode = enumTransforModes(customUI->multihandTypeCombobox->itemData(index).toInt());
    m_configGroup.writeEntry("transformMode", index);


    // hide all of the UI elements by default
    customUI->horizontalCheckbox->setVisible(false);
    customUI->verticalCheckbox->setVisible(false);
    customUI->translationRadiusSpinbox->setVisible(false);
    customUI->radiusLabel->setVisible(false);
    customUI->brushCountSpinBox->setVisible(false);
    customUI->brushesLabel->setVisible(false);
    customUI->subbrushLabel->setVisible(false);
    customUI->addSubbrushButton->setVisible(false);
    customUI->removeSubbrushButton->setVisible(false);

    // turn on what we need
    if (index == MIRROR) {
         customUI->horizontalCheckbox->setVisible(true);
         customUI->verticalCheckbox->setVisible(true);
    }

    else if (index == TRANSLATE) {
        customUI->translationRadiusSpinbox->setVisible(true);
        customUI->radiusLabel->setVisible(true);
        customUI->brushCountSpinBox->setVisible(true);
        customUI->brushesLabel->setVisible(true);
     }

    else if (index == SYMMETRY || index == SNOWFLAKE || index == TRANSLATE ) {
        customUI->brushCountSpinBox->setVisible(true);
        customUI->brushesLabel->setVisible(true);
     }
    
    else if (index == COPYTRANSLATE) {
        customUI->subbrushLabel->setVisible(true);
        customUI->addSubbrushButton->setVisible(true);
        customUI->removeSubbrushButton->setVisible(true);
    }

}

void KisToolMultihand::slotSetAxesVisible(bool vis)
{
    m_showAxes = vis;
    updateCanvas();
    m_configGroup.writeEntry("showAxes", vis);
}


void KisToolMultihand::slotSetMirrorVertically(bool mirror)
{
    m_mirrorVertically = mirror;
    updateCanvas();
    m_configGroup.writeEntry("mirrorVertically", mirror);
}

void KisToolMultihand::slotSetMirrorHorizontally(bool mirror)
{
    m_mirrorHorizontally = mirror;
    updateCanvas();
    m_configGroup.writeEntry("mirrorHorizontally", mirror);
}

void KisToolMultihand::slotSetTranslateRadius(int radius)
{
    m_translateRadius = radius;
    m_configGroup.writeEntry("translateRadius", radius);
}

void KisToolMultihand::slotAddSubbrushesMode(bool checked)
{
    m_addSubbrushesMode = checked;
    updateCanvas();
}

void KisToolMultihand::slotRemoveAllSubbrushes()
{
    m_subbrOriginalLocations.clear();
    updateCanvas();
}

