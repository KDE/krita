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

#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_image.h"
#include "kis_birdeye_box.h"
#include "kis_double_widget.h"

namespace {

    class ViewCanvas : public KoPixelCanvas {
    
    public:
        ViewCanvas(KisCanvasSubject * canvasSubject) : KoPixelCanvas(), m_canvasSubject(canvasSubject) {};
        virtual ~ViewCanvas() {};
        
    public:
    
        virtual QRect visibleArea() 
            { 
                return QRect(0, 0, m_canvasSubject->currentImg()->width(), m_canvasSubject->currentImg()->height()); 
            };
            
        virtual QRect size() 
            { 
                return QRect(0, 0, m_canvasSubject->currentImg()->width(), m_canvasSubject->currentImg()->height()); 
            };
            
        virtual void setViewCenterPoint(Q_INT32 x, Q_INT32 y) 
            { 
                m_canvasSubject->canvasController()->zoomAroundPoint(x, y, m_canvasSubject->zoomFactor());
            };
            
    private:
    
        KisCanvasSubject * m_canvasSubject;
    
    };

    class ViewZoomListener : public KoZoomListener {

        public:

            ViewZoomListener(KisCanvasController * canvasController) 
                : KoZoomListener()
                , m_canvasController(canvasController) {};
            virtual ~ViewZoomListener() {};

        public:

            void zoomTo( Q_INT32 x, Q_INT32 y, double factor ) { m_canvasController->zoomAroundPoint(x, y, factor); }
            void zoomIn() { m_canvasController->zoomIn(); }
            void zoomOut() { m_canvasController->zoomOut(); }
            double getMinZoom() { return (1.0 / 16.0); }
            double getMaxZoom() { return 16.0; }

        private:

            KisCanvasController * m_canvasController;

    };

    class ImageThumbnailProvider : public KoThumbnailProvider {
    
        public:
            ImageThumbnailProvider(KisImageSP image, KisCanvasSubject* canvasSubject)
                : KoThumbnailProvider()
                , m_image(image)
                , m_canvasSubject(canvasSubject) {};
                
            virtual ~ImageThumbnailProvider() {};
            
        public:
        
            virtual QRect pixelSize()
                {
                    return QRect (0, 0, m_image->width(), m_image->height());
                }
                
            virtual QImage image(QRect r)
                {
                    return m_image->convertToQImage(r.x(), r.y(), r.x() + r.width(), r.y() + r.height(), 
                                                    m_canvasSubject->monitorProfile(), 
                                                    m_canvasSubject->HDRExposure());
                }

        private:
        
            KisImageSP m_image;
            KisCanvasSubject * m_canvasSubject;
    
    };

}

KisBirdEyeBox::KisBirdEyeBox(KisCanvasSubject * canvasSubject, QWidget* parent, const char* name)
    : QWidget(parent, name)
    , m_canvasSubject(canvasSubject)
{
    QVBoxLayout * l = new QVBoxLayout(this);

    KoZoomListener * kzl = new ViewZoomListener(canvasSubject->canvasController());
    KoThumbnailProvider * ktp = new ImageThumbnailProvider(canvasSubject->currentImg(), canvasSubject);
    KoPixelCanvas * kpc = new ViewCanvas(m_canvasSubject);

    m_birdEyePanel = new KoBirdEyePanel(kzl, ktp, kpc, this);
    l->addWidget(m_birdEyePanel);

    QHBoxLayout * hl = new QHBoxLayout(l);

    hl->addWidget(new QLabel(i18n("Exposure:"), this));

    m_exposureDoubleWidget = new KisDoubleWidget(-10, 10, this);
    hl->addWidget(m_exposureDoubleWidget);

    l->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

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
        m_canvasSubject->setHDRExposure(exposure);
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
