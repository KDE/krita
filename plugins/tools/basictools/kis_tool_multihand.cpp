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
      m_setupAxesFlag(false)
{
    m_helper =
        new KisToolMultihandHelper(paintingInformationBuilder(),
                                   kundo2_i18n("Multibrush Stroke"),
                                   recordingAdapter());
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

void KisToolMultihand::paint(QPainter& gc, const KoViewConverter &converter)
{
    if(m_setupAxesFlag) {
        int diagonal = (currentImage()->height() + currentImage()->width());

        QPainterPath path;
        path.moveTo(m_axesPoint.x()-diagonal*cos(m_angle), m_axesPoint.y()-diagonal*sin(m_angle));
        path.lineTo(m_axesPoint.x()+diagonal*cos(m_angle), m_axesPoint.y()+diagonal*sin(m_angle));
        path.moveTo(m_axesPoint.x()-diagonal*cos(m_angle+M_PI_2), m_axesPoint.y()-diagonal*sin(m_angle+M_PI_2));
        path.lineTo(m_axesPoint.x()+diagonal*cos(m_angle+M_PI_2), m_axesPoint.y()+diagonal*sin(m_angle+M_PI_2));
        paintToolOutline(&gc, pixelToView(path));
    }
    else {
        KisToolFreehand::paint(gc, converter);
        if(m_showAxes){
            int diagonal = (currentImage()->height() + currentImage()->width());

            QPainterPath path;
            path.moveTo(m_axesPoint.x()-diagonal*cos(m_angle), m_axesPoint.y()-diagonal*sin(m_angle));
            path.lineTo(m_axesPoint.x()+diagonal*cos(m_angle), m_axesPoint.y()+diagonal*sin(m_angle));
            path.moveTo(m_axesPoint.x()-diagonal*cos(m_angle+M_PI_2), m_axesPoint.y()-diagonal*sin(m_angle+M_PI_2));
            path.lineTo(m_axesPoint.x()+diagonal*cos(m_angle+M_PI_2), m_axesPoint.y()+diagonal*sin(m_angle+M_PI_2));
            paintToolOutline(&gc, pixelToView(path));
        }
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
    else /* if(m_transformationNode == TRANSLATE) */ {
        /**
         * TODO: currently, the seed is the same for all the
         * strokes
         */
#ifdef Q_OS_WIN
        srand(0);
#else
        srand48(0);
#endif

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
    }

    m_helper->setupTransformations(transformations);
}

QWidget* KisToolMultihand::createOptionWidget()
{
    QWidget *widget = KisToolBrush::createOptionWidget();

    m_axesChCkBox = new QCheckBox(i18n("Show Axes"));

    connect(m_axesChCkBox,SIGNAL(toggled(bool)),this, SLOT(slotSetAxesVisible(bool)));

    m_axesPointBtn = new QPushButton(i18n("Axes point"), widget);
    m_axesPointBtn->setCheckable(true);
    connect(m_axesPointBtn, SIGNAL(clicked(bool)),this, SLOT(activateAxesPointModeSetup()));
    addOptionWidgetOption(m_axesPointBtn, m_axesChCkBox);

    m_axesAngleSlider = new KisDoubleSliderSpinBox(widget);
    m_axesAngleSlider->setToolTip(i18n("Set axes angle (degrees)"));
    m_axesAngleSlider->setSuffix(QChar(Qt::Key_degree));
    m_axesAngleSlider->setRange(0.0, 90.0,1);

    m_axesAngleSlider->setEnabled(true);
    connect(m_axesAngleSlider, SIGNAL(valueChanged(qreal)),this, SLOT(slotSetAxesAngle(qreal)));
    addOptionWidgetOption(m_axesAngleSlider, new QLabel(i18n("Axes Angle:")));

    m_transformModesComboBox = new QComboBox(widget);
    m_transformModesComboBox->addItem(i18n("Symmetry"),int(SYMMETRY));
    m_transformModesComboBox->addItem(i18n("Mirror"),int(MIRROR));
    m_transformModesComboBox->addItem(i18n("Translate"),int(TRANSLATE));
    m_transformModesComboBox->addItem(i18n("Snowflake"),int(SNOWFLAKE));

    connect(m_transformModesComboBox,SIGNAL(currentIndexChanged(int)),SLOT(slotSetTransformMode(int)));
    addOptionWidgetOption(m_transformModesComboBox);

    m_handsCountSlider = new KisSliderSpinBox(widget);
    m_handsCountSlider->setToolTip(i18n("Brush count"));
    m_handsCountSlider->setRange(1, MAXIMUM_BRUSHES);
    m_handsCountSlider->setEnabled(true);
    connect(m_handsCountSlider, SIGNAL(valueChanged(int)),this, SLOT(slotSetHandsCount(int)));
    addOptionWidgetOption(m_handsCountSlider);

    m_modeCustomOption = new QStackedWidget(widget);

    QWidget * symmetryWidget = new QWidget(m_modeCustomOption);
    m_modeCustomOption->addWidget(symmetryWidget);

    QWidget * mirrorWidget = new QWidget(m_modeCustomOption);
    m_mirrorHorizontallyChCkBox = new QCheckBox(i18n("Horizontally"));

    m_mirrorVerticallyChCkBox = new QCheckBox(i18n("Vertically"));

    connect(m_mirrorHorizontallyChCkBox,SIGNAL(toggled(bool)),this, SLOT(slotSetMirrorHorizontally(bool)));
    connect(m_mirrorVerticallyChCkBox,SIGNAL(toggled(bool)),this, SLOT(slotSetMirrorVertically(bool)));

    QGridLayout * mirrorLayout = new QGridLayout(mirrorWidget);
    mirrorLayout->addWidget(m_mirrorHorizontallyChCkBox,0,0);
    mirrorLayout->addWidget(m_mirrorVerticallyChCkBox,0,1);
    mirrorWidget->setLayout(mirrorLayout);
    m_modeCustomOption->addWidget(mirrorWidget);

    QWidget * translateWidget = new QWidget(m_modeCustomOption);
    m_translateRadiusSlider = new KisSliderSpinBox(translateWidget);
    m_translateRadiusSlider->setRange(0, 200);

    m_translateRadiusSlider->setSuffix(i18n(" px"));
    connect(m_translateRadiusSlider,SIGNAL(valueChanged(int)),this,SLOT(slotSetTranslateRadius(int)));

    QFormLayout *radiusLayout = new QFormLayout(translateWidget);
    radiusLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    radiusLayout->addRow(i18n("Radius"), m_translateRadiusSlider);
    translateWidget->setLayout(radiusLayout);

    m_modeCustomOption->addWidget(translateWidget);
    m_modeCustomOption->setCurrentIndex(m_transformModesComboBox->currentIndex());

    addOptionWidgetOption(m_modeCustomOption);


    // read values from configuration file
    m_axesChCkBox->setChecked((bool)m_configGroup.readEntry("showAxes", false));
    m_mirrorHorizontallyChCkBox->setChecked((bool)m_configGroup.readEntry("mirrorHorizontally", false));
    m_mirrorVerticallyChCkBox->setChecked((bool)m_configGroup.readEntry("mirrorVertically", false));
    m_axesAngleSlider->setValue(m_configGroup.readEntry("axesAngle", 0.0));
    m_transformModesComboBox->setCurrentIndex(m_configGroup.readEntry("transformMode", 0));
    m_translateRadiusSlider->setValue(m_configGroup.readEntry("translateRadius", 0));
    m_handsCountSlider->setValue(m_configGroup.readEntry("handsCount", 4));

    return widget;
}

void KisToolMultihand::activateAxesPointModeSetup()
{
    if (m_axesPointBtn->isChecked()){
        m_setupAxesFlag = true;
        useCursor(KisCursor::crossCursor());
        updateCanvas();
    } else {
        finishAxesSetup();
    }
}

void KisToolMultihand::finishAxesSetup()
{
    m_setupAxesFlag = false;
    m_axesPointBtn->setChecked(false);
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
    m_transformMode = enumTransforModes(m_transformModesComboBox->itemData(index).toInt());
    m_modeCustomOption->setCurrentIndex(index);
    m_handsCountSlider->setVisible(m_transformMode != MIRROR);
    m_configGroup.writeEntry("transformMode", index);
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
    m_configGroup.writeEntry("mirrorVertically", mirror);
}

void KisToolMultihand::slotSetMirrorHorizontally(bool mirror)
{
    m_mirrorHorizontally = mirror;
    m_configGroup.writeEntry("mirrorHorizontally", mirror);
}

void KisToolMultihand::slotSetTranslateRadius(int radius)
{
    m_translateRadius = radius;
    m_configGroup.writeEntry("translateRadius", radius);
}

