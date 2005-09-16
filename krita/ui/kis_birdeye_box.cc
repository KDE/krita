/*
 *  Copyright (c) 2004 Kivio Team
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

#include "qlayout.h"
#include "qlabel.h"
#include "qpixmap.h"
#include "qpainter.h"

#include "klocale.h"

#include "kobirdeyepanel.h"

#include "kis_view.h"
#include "kis_image.h"
#include "kis_birdeye_box.h"
#include "kis_double_widget.h"

namespace {

    class ViewZoomListener : public KoZoomListener {

        public:

            ViewZoomListener(KisView * view) : KoZoomListener(), m_view(view) {};
            virtual ~ViewZoomListener() {};

        public:

            void zoomTo(int ) {}
            void zoomIn() { m_view->slotZoomIn(); }
            void zoomOut() { m_view->slotZoomOut(); }
            int getMinZoom() { return 10; }
            int getMaxZoom() { return 1600; }

        private:

            KisView * m_view;

    };

    class ImageThumbnailProvider : public KoThumbnailProvider {
    
        public:
            ImageThumbnailProvider(KisImageSP image, KisView * view) : KoThumbnailProvider(), m_image(image), m_view(view) {};
            virtual ~ImageThumbnailProvider() {};
            
        public:
        
            virtual QRect pixelSize()
                {
                    return QRect (0, 0, m_image->width(), m_image->height());
                }
                
            virtual QPixmap image(QRect r)
                {
                    QPixmap px (r.width(), r.height());
                    QPainter p(&px);
                    m_image->renderToPainter(r.x(), r.y(), r.width(), r.height(), p, m_view->monitorProfile(), m_view->HDRExposure());
                    return px;
                }

        private:
        
            KisImageSP m_image;
            KisView * m_view;
    
    };

}

KisBirdEyeBox::KisBirdEyeBox(KisView * view, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    QVBoxLayout * l = new QVBoxLayout(this);
    
    KoZoomListener * kzl = new ViewZoomListener(view);
    KoThumbnailProvider * ktp = new ImageThumbnailProvider(view->currentImg(), view);
    
    m_birdEyePanel = new KoBirdEyePanel(kzl, ktp, this);
    l->addWidget(m_birdEyePanel);
    
    QHBoxLayout * hl = new QHBoxLayout(l);

    hl->addWidget(new QLabel(i18n("Exposure:"), this));
    
    m_exposureDoubleWidget = new KisDoubleWidget(-10, 10, this);
    hl->addWidget(m_exposureDoubleWidget);
    
    m_exposureDoubleWidget -> setPrecision(1);
    m_exposureDoubleWidget -> setValue(0);
    m_exposureDoubleWidget -> setLineStep(0.1);
    m_exposureDoubleWidget -> setPageStep(1);

    connect(m_exposureDoubleWidget, SIGNAL(valueChanged(double)), SLOT(exposureValueChanged(double)));
    connect(m_exposureDoubleWidget, SIGNAL(sliderPressed()), SLOT(exposureSliderPressed()));
    connect(m_exposureDoubleWidget, SIGNAL(sliderReleased()), SLOT(exposureSliderReleased()));
    
    m_draggingExposureSlider = false;
}

KisBirdEyeBox::~KisBirdEyeBox()
{
}

void KisBirdEyeBox::exposureValueChanged(double exposure)
{
    if (!m_draggingExposureSlider) {
        emit exposureChanged(static_cast<float>(exposure));
    }
}

void KisBirdEyeBox::exposureSliderPressed()
{
    m_draggingExposureSlider = true;
}

void KisBirdEyeBox::exposureSliderReleased()
{
    m_draggingExposureSlider = false;
    exposureValueChanged(m_exposureDoubleWidget -> value());
}

#include "kis_birdeye_box.moc"
