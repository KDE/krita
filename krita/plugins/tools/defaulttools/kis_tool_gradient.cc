/*
 *  kis_tool_gradient.cc - part of Krita
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004-2007 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_tool_gradient.h"

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
#include <GL/glew.h>
#include <QGLWidget>
#endif

#include <cfloat>

#include <QApplication>
#include <QPainter>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>

#include <kis_transaction.h>
#include <kis_debug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kcombobox.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoUpdater.h>
#include <KoProgressUpdater.h>

#include <kis_gradient_painter.h>
#include <kis_gradient_job.h>
#include <kis_painter.h>
#include <kis_canvas_resource_provider.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_threaded_applicator.h>
#include <kis_paint_layer.h>

#include <canvas/kis_canvas2.h>
#include <kis_view2.h>
#include <widgets/kis_cmb_composite.h>
#include <widgets/kis_double_widget.h>
#include <kis_cursor.h>
#include <kis_config.h>

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
#include <opengl/kis_opengl.h>
#include <opengl/kis_opengl_gradient_program.h>
#include <opengl/kis_opengl_canvas2.h>
#include <canvas/kis_canvas2.h>
#include <KoSliderCombo.h>
#include <kis_config_notifier.h>
#endif

KisToolGradient::KisToolGradient(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_gradient_cursor.png", 6, 6)),
        m_dragging(false)
{
    setObjectName("tool_gradient");

    m_startPos = QPointF(0, 0);
    m_endPos = QPointF(0, 0);

    m_reverse = false;
    m_shape = KisGradientPainter::GradientShapeLinear;
    m_repeat = KisGradientPainter::GradientRepeatNone;
    m_antiAliasThreshold = 0.2;

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
    m_gradientProgram = 0;
    m_previewOpacityPercent = 75;
#endif
}

KisToolGradient::~KisToolGradient()
{
}

void KisToolGradient::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_dragging && m_startPos != m_endPos) {

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
        if (m_gradientProgram) {

            QPointF gradientVector = m_endPos - m_startPos;
            double gradientVectorLength = sqrt((gradientVector.x() * gradientVector.x()) + (gradientVector.y() * gradientVector.y()));

            if (gradientVectorLength > DBL_EPSILON) {
                QPointF normalisedGradientVector;

                normalisedGradientVector.rx() = gradientVector.x() / gradientVectorLength;
                normalisedGradientVector.ry() = gradientVector.y() / gradientVectorLength;

                QPointF normalisedGradientVectorStart = m_startPos;
                normalisedGradientVectorStart /= gradientVectorLength;

                KisOpenGLCanvas2 *canvasWidget = dynamic_cast<KisOpenGLCanvas2 *>(m_canvas->canvasWidget());
                Q_ASSERT(canvasWidget);

                if (canvasWidget) {
                    canvasWidget->setPixelToViewTransformation();

                    glMatrixMode(GL_MODELVIEW);
                    glTranslatef(0.5, 0.5, 0.0);

                    glMatrixMode(GL_TEXTURE);
                    glLoadIdentity();
                    glScalef(1 / gradientVectorLength, 1 / gradientVectorLength, 1);

                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                    canvasWidget->makeCurrent();
                    m_gradientProgram->activate(normalisedGradientVectorStart,
                                                normalisedGradientVectorStart + normalisedGradientVector);

                    //                     glValidateProgramARB(m_gradientProgram->handle());
                    //                     dbgTools <<"Validate:";
                    //                     dbgTools << m_gradientProgram->getInfoLog();


                    glBegin(GL_QUADS);

                    glTexCoord2f(0.0, 0.0);
                    glVertex2f(0.0, 0.0);

                    glTexCoord2f(currentImage()->width(), 0.0);
                    glVertex2f(currentImage()->width(), 0.0);

                    glTexCoord2f(currentImage()->width(), currentImage()->height());
                    glVertex2f(currentImage()->width(), currentImage()->height());

                    glTexCoord2f(0.0, currentImage()->height());
                    glVertex2f(0.0, currentImage()->height());

                    glEnd();

                    KisOpenGLProgram::deactivate();

                    glDisable(GL_BLEND);

                    // Unbind the texture otherwise the ATI driver crashes when the canvas context is
                    // made current after the textures are deleted following an image resize.
                    glBindTexture(GL_TEXTURE_1D, 0);
                }
            }
        } else
#endif
        {
            qreal sx, sy;
            converter.zoom(&sx, &sy);
            painter.scale(sx / currentImage()->xRes(), sy / currentImage()->yRes());
            paintLine(painter);
        }
    }
}

void KisToolGradient::mousePressEvent(KoPointerEvent *e)
{
    if (!currentImage()) {
        return;
    }

    QPointF pos = convertToPixelCoord(e);

    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_startPos = pos;
        m_endPos = pos;

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
        KisConfig cfg;

        if (cfg.useOpenGL() && cfg.useOpenGLShaders()) {

            KisCanvas2 *canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
            KoColorProfile *monitorProfile = 0;

            Q_ASSERT(canvas);

            if (canvas) {
                monitorProfile = canvas->monitorProfile();
            }

            KisOpenGL::makeContextCurrent();
            m_gradientProgram = new KisOpenGLGradientProgram(currentGradient(),
                    m_shape,
                    m_repeat,
                    m_reverse,
                    currentImage()->colorSpace(),
                    monitorProfile,
                    m_previewOpacityPercent / 100.0);
        }
#endif
    }
}

void KisToolGradient::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_dragging) {
        QPointF pos = convertToPixelCoord(e);

        QRectF bound;
        bound.setTopLeft(m_startPos);
        bound.setBottomRight(m_endPos);
        m_canvas->updateCanvas(convertToPt(bound.normalized()));

        if ((e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier) {
            m_endPos = straightLine(pos);
        } else {
            m_endPos = pos;
        }

        bound.setTopLeft(m_startPos);
        bound.setBottomRight(m_endPos);
        m_canvas->updateCanvas(convertToPt(bound.normalized()));
    }
}

void KisToolGradient::mouseReleaseEvent(KoPointerEvent *e)
{
    if (!currentNode())
       return;

    if (m_dragging && e->button() == Qt::LeftButton) {

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
        delete m_gradientProgram;
        m_gradientProgram = 0;
#endif

        m_dragging = false;

        if (m_startPos == m_endPos) {
            m_dragging = false;
            return;
        }

        QPointF pos = convertToPixelCoord(e);

        if ((e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier) {
            m_endPos = straightLine(pos);
        } else {
            m_endPos = pos;
        }

        KisPaintDeviceSP device;

        if (currentImage() && (device = currentNode()->paintDevice())) {

#if 1 // unthreaded
            qApp->setOverrideCursor(Qt::BusyCursor);

            KisGradientPainter painter(device, currentSelection());
            if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(currentNode().data())) {
                painter.setChannelFlags(l->channelFlags());
                if (l->alphaLocked()) {
                    painter.setLockAlpha(l->alphaLocked());
                }
            }

            painter.beginTransaction(i18n("Gradient"));

            painter.setPaintColor(currentFgColor());
            painter.setGradient(currentGradient());
            painter.setOpacity(m_opacity);
            painter.setCompositeOp(m_compositeOp);

            KisCanvas2 * canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
            KoProgressUpdater * updater = canvas->view()->createProgressUpdater(KoProgressUpdater::Unthreaded);

            updater->start(100, i18n("Gradient"));
            painter.setProgress(updater->startSubtask());

            painter.paintGradient(m_startPos, m_endPos, m_shape, m_repeat, m_antiAliasThreshold, m_reverse, 0, 0, currentImage()->width(), currentImage()->height());
            m_canvas->addCommand(painter.endTransaction());

            qApp->restoreOverrideCursor();
#else
            // XXX: figure out why threaded gradients give weird noise
            KisTransaction* transaction = new KisTransaction(i18n("Gradient"), device);

            KisCanvas2 * canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
            KoProgressUpdater * updater = canvas->view()->createProgressUpdater();
            updater->start(100, i18n("Gradient"));

            KisGradientPainter::Configuration config;
            config.gradient = currentGradient();
            config.transaction = transaction;
            config.fgColor = currentFgColor();
            config.opacity = m_opacity;
            config.compositeOp = m_compositeOp;
            config.vectorStart = m_startPos;
            config.vectorEnd = m_endPos;
            config.shape = m_shape;
            config.repeat = m_repeat;
            config.antiAliasThreshold = m_antiAliasThreshold;
            config.reverse = m_reverse;

            KisGradientJobFactory factory(&config, currentSelection());
            KisThreadedApplicator applicator(device, currentImage()->bounds(), &factory, updater);
            connect(&applicator, SIGNAL(areaDone(const QRect&)), this, SLOT(areaDone(const QRect&)));

            applicator.execute();

            m_canvas->addCommand(transaction);
#endif
            currentNode()->setDirty();
            notifyModified();
            delete updater;
        }
        m_canvas->updateCanvas(convertToPt(currentImage()->bounds()));

    }
}

QPointF KisToolGradient::straightLine(QPointF point)
{
    QPointF comparison = point - m_startPos;
    QPointF result;

    if (fabs(comparison.x()) > fabs(comparison.y())) {
        result.setX(point.x());
        result.setY(m_startPos.y());
    } else {
        result.setX(m_startPos.x());
        result.setY(point.y());
    }

    return result;
}

void KisToolGradient::paintLine(QPainter& gc)
{
    if (m_canvas) {
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);

        gc.setPen(pen);
        gc.drawLine(m_startPos, m_endPos);
        gc.setPen(old);
    }
}

QWidget* KisToolGradient::createOptionWidget()
{
    QWidget *widget = KisToolPaint::createOptionWidget();
    Q_CHECK_PTR(widget);
    widget->setObjectName(toolId() + " option widget");

    m_lbShape = new QLabel(i18n("Shape:"), widget);
    m_lbRepeat = new QLabel(i18n("Repeat:"), widget);

    m_ckReverse = new QCheckBox(i18nc("the gradient will be drawn with the color order reversed", "Reverse"), widget);
    m_ckReverse->setObjectName("reverse_check");
    connect(m_ckReverse, SIGNAL(toggled(bool)), this, SLOT(slotSetReverse(bool)));

    m_cmbShape = new KComboBox(widget);
    m_cmbShape->setObjectName("shape_combo");
    connect(m_cmbShape, SIGNAL(activated(int)), this, SLOT(slotSetShape(int)));
    m_cmbShape->addItem(i18nc("the gradient will be drawn linearly", "Linear"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn bilinearly", "Bi-Linear"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn radially", "Radial"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn in a square around a centre", "Square"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn as an assymmetric cone", "Conical"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn as a symmetric cone", "Conical Symmetric"));

    m_cmbRepeat = new KComboBox(widget);
    m_cmbRepeat->setObjectName("repeat_combo");
    connect(m_cmbRepeat, SIGNAL(activated(int)), this, SLOT(slotSetRepeat(int)));
    m_cmbRepeat->addItem(i18nc("The gradient will not repeat", "None"));
    m_cmbRepeat->addItem(i18nc("The gradient will repeat forwards", "Forwards"));
    m_cmbRepeat->addItem(i18nc("The gradient will repeat alternatingly", "Alternating"));

    addOptionWidgetOption(m_cmbShape, m_lbShape);

    addOptionWidgetOption(m_cmbRepeat, m_lbRepeat);

    addOptionWidgetOption(m_ckReverse);

    m_lbAntiAliasThreshold = new QLabel(i18n("Anti-alias threshold:"), widget);

    m_slAntiAliasThreshold = new KDoubleNumInput(widget);
    m_slAntiAliasThreshold->setObjectName("threshold_slider");
    m_slAntiAliasThreshold->setMinimum(0);
    m_slAntiAliasThreshold->setMaximum(1);
    m_slAntiAliasThreshold->setValue(m_antiAliasThreshold);
    connect(m_slAntiAliasThreshold, SIGNAL(valueChanged(double)), this, SLOT(slotSetAntiAliasThreshold(double)));

    addOptionWidgetOption(m_slAntiAliasThreshold, m_lbAntiAliasThreshold);

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
    m_lbPreviewOpacity = new QLabel(i18n("Preview opacity:"), widget);
    m_slPreviewOpacity = new KoSliderCombo(widget);
    m_slPreviewOpacity->setDecimals(0);
    m_slPreviewOpacity->setValue(m_previewOpacityPercent);
    connect(m_slPreviewOpacity, SIGNAL(valueChanged(qreal, bool)), this, SLOT(slotSetPreviewOpacity(qreal, bool)));

    addOptionWidgetOption(m_slPreviewOpacity, m_lbPreviewOpacity);

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();
#endif
    widget->setFixedHeight(widget->sizeHint().height());
    return widget;
}

void KisToolGradient::slotSetShape(int shape)
{
    m_shape = static_cast<KisGradientPainter::enumGradientShape>(shape);
}

void KisToolGradient::slotSetRepeat(int repeat)
{
    m_repeat = static_cast<KisGradientPainter::enumGradientRepeat>(repeat);
}

void KisToolGradient::slotSetReverse(bool state)
{
    m_reverse = state;
}

void KisToolGradient::slotSetAntiAliasThreshold(double value)
{
    m_antiAliasThreshold = value;
}

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)

void KisToolGradient::slotSetPreviewOpacity(qreal value, bool final)
{
    Q_UNUSED(final);
    m_previewOpacityPercent = value;
}

void KisToolGradient::slotConfigChanged()
{
    KisConfig cfg;
    bool enablePreviewOpacity = cfg.useOpenGL() && cfg.useOpenGLShaders();

    m_slPreviewOpacity->setEnabled(enablePreviewOpacity);
    m_lbPreviewOpacity->setEnabled(enablePreviewOpacity);
}

#endif

#include "kis_tool_gradient.moc"

