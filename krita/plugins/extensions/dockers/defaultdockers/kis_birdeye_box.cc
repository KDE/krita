/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_birdeye_box.h"

#include <lcms.h>

#include <QLayout>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <klocale.h>

#include <KoColorSpace.h>

#include "kis_view2.h"
#include "kis_doc2.h"

#include "widgets/kis_double_widget.h"
#include "canvas/kis_canvas2.h"
#include "kis_image.h"
#include "kis_iterators_pixel.h"
#include "kis_canvas_resource_provider.h"

namespace
{
#if 0 // XXX: Redo when zooming is implemented again (BSAR)

class CanvasAdapter : public KoCanvasAdapter
{

public:
    CanvasAdapter(KisCanvasSubject * canvasSubject) : KoCanvasAdapter(), m_canvasSubject(canvasSubject) {}
    virtual ~CanvasAdapter() {}

public:

    virtual QRectF visibleArea() {
        if (!m_canvasSubject->currentImg()) return QRectF(0, 0, 0, 0);

        KisCanvasController * c = m_canvasSubject->canvasController();

        if (c && c->kiscanvas())
            return c->viewToWindow(KisRect(0, 0, c->kiscanvas()->width(), c->kiscanvas()->height()));
        else
            return QRectF(0, 0, 0, 0);
    }

    virtual double zoomFactor() {
        return m_canvasSubject->zoomFactor();
    }

    virtual QRect size() {
        if (!m_canvasSubject->currentImg()) return QRect(0, 0, 0, 0);

        return QRect(0, 0, m_canvasSubject->currentImg()->width(), m_canvasSubject->currentImg()->height());
    }

    virtual void setViewCenterPoint(double x, double y) {
        m_canvasSubject->canvasController()->zoomAroundPoint(x, y, m_canvasSubject->zoomFactor());
    }

private:

    KisCanvasSubject * m_canvasSubject;

};

class ZoomListener : public KoZoomAdapter
{

public:

    ZoomListener(KisCanvasController * canvasController)
            : KoZoomAdapter()
            , m_canvasController(canvasController) {}
    virtual ~ZoomListener() {}

public:

    void zoomTo(double x, double y, double factor) {
        m_canvasController->zoomAroundPoint(x, y, factor);
    }
    void zoomIn() {
        m_canvasController->zoomIn();
    }
    void zoomOut() {
        m_canvasController->zoomOut();
    }
    double getMinZoom() {
        return (1.0 / 500);
    }
    double getMaxZoom() {
        return 16.0;
    }

private:

    KisCanvasController * m_canvasController;

};

class ThumbnailProvider : public KoThumbnailAdapter
{

public:
    ThumbnailProvider(KisImageWSP image, KisCanvasSubject* canvasSubject)
            : KoThumbnailAdapter()
            , m_image(image)
            , m_canvasSubject(canvasSubject) {}

    virtual ~ThumbnailProvider() {}

public:

    virtual QSize pixelSize() {
        if (!m_image) return QSize(0, 0);
        return QSize(m_image->width(), m_image->height());
    }

    virtual QImage image(const QRect & r, const QSize & thumbnailSize) {
        if (!m_image || r.isEmpty() || thumbnailSize.width() == 0 || thumbnailSize.height() == 0) {
            return QImage();
        }

        KisPaintDevice thumbnailRect(m_image->colorSpace(), "thumbnailRect");
        KisPaintDeviceSP mergedImage = m_image->projection();

        qint32 imageWidth = m_image->width();
        qint32 imageHeight = m_image->height();
        quint32 pixelSize = m_image->colorSpace()->pixelSize();

        KisHLineIteratorPixel it = thumbnailRect.createHLineIterator(0, 0, r.width());
        for (qint32 y = 0; y < r.height(); ++y) {

            qint32 thumbnailY = r.y() + y;
            qint32 thumbnailX = r.x();
            qint32 imageY = (thumbnailY * imageHeight) / thumbnailSize.height();
            KisHLineConstIteratorPixel srcIt = mergedImage -> createHLineIterator(0, imageY, imageWidth);

            while (!it.isDone()) {

                qint32 imageX = (thumbnailX * imageWidth) / thumbnailSize.width();
                qint32 dx = imageX - srcIt.x();
                srcIt += dx;

                //KoColor pixelColor = mergedImage->colorAt(imageX, imageY);
                memcpy(it.rawData(), srcIt.rawData(), pixelSize);

                ++it;
                ++thumbnailX;
            }
            it.nextRow();
        }

        return thumbnailRect.convertToQImage(m_canvasSubject->monitorProfile(), 0, 0, r.width(), r.height(),
                                             m_canvasSubject->HDRExposure());
    }

    void setImage(KisImageWSP image) {
        m_image = image;
    }
private:

    KisImageWSP m_image;
    KisCanvasSubject * m_canvasSubject;

};
#endif
}

KisBirdEyeBox::KisBirdEyeBox(KisView2 * view)
        : QDockWidget(i18n("Overview"))
        , m_view(view)
{

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget * w = new QWidget(this);
    setWidget(w);

    QVBoxLayout * l = new QVBoxLayout(w);

    m_image = m_view->image();
#if 0 // XXX: Redo when zooming is implemented again (BSAR)
    m_zoomAdapter = new ZoomListener(m_subject->canvasController()); // The birdeye panel deletes
    KoThumbnailAdapter * ktp = new ThumbnailProvider(m_image, m_subject);  // The birdeye panel deletes
    KoCanvasAdapter * kpc = new CanvasAdapter(m_subject);  // The birdeye panel deletes

    m_birdEyePanel = new KoBirdEyePanel(m_zoomAdapter, ktp, kpc, this);

    connect(view, SIGNAL(cursorPosition(qint32, qint32)), m_birdEyePanel, SLOT(cursorPosChanged(qint32, qint32)));
    connect(view, SIGNAL(viewTransformationsChanged()), m_birdEyePanel, SLOT(slotViewTransformationChanged()));

    l->addWidget(m_birdEyePanel);
#endif

    QHBoxLayout * hl = new QHBoxLayout();
    l->addLayout(hl);

    m_exposureLabel = new QLabel(i18n("Exposure:"), w);
    hl->addWidget(m_exposureLabel);

    m_exposureDoubleWidget = new KisDoubleWidget(-10, 10, w);
    m_exposureDoubleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_exposureDoubleWidget->setToolTip("Select the exposure (stops) for HDR images");
    hl->addWidget(m_exposureDoubleWidget);

    l->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

    m_exposureDoubleWidget->setPrecision(1);
    m_exposureDoubleWidget->setValue(0);
    m_exposureDoubleWidget->setSingleStep(0.1);
    m_exposureDoubleWidget->setPageStep(1);

    connect(m_exposureDoubleWidget, SIGNAL(valueChanged(double)), SLOT(exposureValueChanged(double)));
    connect(m_exposureDoubleWidget, SIGNAL(sliderPressed()), SLOT(exposureSliderPressed()));
    connect(m_exposureDoubleWidget, SIGNAL(sliderReleased()), SLOT(exposureSliderReleased()));

    m_draggingExposureSlider = false;
    if (m_image) {
        connect(m_image.data(), SIGNAL(sigImageUpdated(QRect)), SLOT(slotImageUpdated(QRect)));
    }

}

KisBirdEyeBox::~KisBirdEyeBox()
{
}

void KisBirdEyeBox::setImage(KisImageWSP image)
{
    if (m_image) {
        m_image->disconnect(this);
    }

    m_image = image;
#if 0 // XXX: Redo when zooming is implemented again (BSAR)
    KoThumbnailAdapter * ktp = new ThumbnailProvider(m_image, m_subject);
    m_birdEyePanel->setThumbnailProvider(ktp);

    if (m_image) {
        connect(m_image.data(), SIGNAL(sigImageUpdated(QRect)), SLOT(slotImageUpdated(QRect)));
        connect(m_image.data(), SIGNAL(sigSizeChanged(qint32, qint32)), SLOT(slotImageSizeChanged(qint32, qint32)));
        connect(m_image.data(), SIGNAL(sigColorSpaceChanged(const KoColorSpace *)), SLOT(slotImageColorSpaceChanged(const KoColorSpace *)));
        m_birdEyePanel->slotUpdate(m_image->bounds());
        slotImageColorSpaceChanged(m_image->colorSpace());
    }
#endif
}

void KisBirdEyeBox::slotDocCommandExecuted()
{
#if 0 // XXX: Redo when zooming is implemented again (BSAR)
    if (m_image) {
        if (!m_dirtyRegion.isEmpty()) {
            m_birdEyePanel->slotUpdate(m_dirtyRegion);
        }
        m_dirtyRegion = QRect();
    }
#endif
}

void KisBirdEyeBox::slotImageUpdated(const QRect & r)
{
    m_dirtyRegion |= r;
}

void KisBirdEyeBox::slotImageSizeChanged(qint32 /*w*/, qint32 /*h*/)
{
#if 0 // XXX: Redo when zooming is implemented again (BSAR)
    if (m_image) {
        m_birdEyePanel->slotUpdate(m_image->bounds());
    }
#endif
}

void KisBirdEyeBox::slotImageColorSpaceChanged(const KoColorSpace *cs)
{
    if (cs->hasHighDynamicRange()) {
        m_exposureDoubleWidget->show();
        m_exposureLabel->show();
    } else {
        m_exposureDoubleWidget->hide();
        m_exposureLabel->hide();
    }
}

void KisBirdEyeBox::exposureValueChanged(double exposure)
{
    if (!m_draggingExposureSlider || m_view->canvasBase()->usingHDRExposureProgram()) {
        m_view->resourceProvider()->setHDRExposure(exposure);

#if 0 // XXX: Redo when zooming is implemented again (BSAR)
        if (m_image && m_image->colorSpace()->hasHighDynamicRange()) {
            m_birdEyePanel->slotUpdate(m_image->bounds());
        }
#endif
    }
}

void KisBirdEyeBox::exposureSliderPressed()
{
    m_draggingExposureSlider = true;
}

void KisBirdEyeBox::exposureSliderReleased()
{
    m_draggingExposureSlider = false;
    exposureValueChanged(m_exposureDoubleWidget->value());
}

#include "kis_birdeye_box.moc"
