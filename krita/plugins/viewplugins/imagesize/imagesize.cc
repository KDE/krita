/*
 * imagesize.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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


#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qrect.h>
#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstdaction.h>

#include <kis_doc.h>
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_transaction.h>
#include <kis_canvas_controller.h>
#include <kis_scale_visitor.h>

#include "imagesize.h"
#include "dlg_imagesize.h"
#include "dlg_layersize.h"
#include "kis_filter_strategy.h"

typedef KGenericFactory<ImageSize> ImageSizeFactory;
K_EXPORT_COMPONENT_FACTORY( kritaimagesize, ImageSizeFactory( "krita" ) )

ImageSize::ImageSize(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    if ( parent->inherits("KisView") )
    {
        setInstance(ImageSizeFactory::instance());
        setXMLFile(locate("data","kritaplugins/imagesize.rc"), true);

        (void) new KAction(i18n("Change &Image Size..."), 0, "Shift-s", this, SLOT(slotImageSize()), actionCollection(), "imagesize");
        (void) new KAction(i18n("&Scale Layer..."), 0, 0, this, SLOT(slotLayerSize()), actionCollection(), "layerscale");


        m_view = (KisView*) parent;
        // Selection manager takes ownership?
        KAction * a = new KAction(i18n("&Scale Selection..."), 0, 0, this, SLOT(slotSelectionScale()), actionCollection(), "selectionscale");
        Q_CHECK_PTR(a);
        m_view ->canvasSubject()-> selectionManager()->addSelectionAction(a);
    }
}

ImageSize::~ImageSize()
{
    m_view = 0;
}

void ImageSize::slotImageSize()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgImageSize * dlgImageSize = new DlgImageSize(m_view, "ImageSize");
    Q_CHECK_PTR(dlgImageSize);

    dlgImageSize->setCaption(i18n("Image Size"));

    KisConfig cfg;

    dlgImageSize->setWidth(image->width());
    dlgImageSize->setHeight(image->height());

    if (dlgImageSize->exec() == QDialog::Accepted) {
        Q_INT32 w = dlgImageSize->width();
        Q_INT32 h = dlgImageSize->height();

        if (dlgImageSize->scale()) {
            m_view->scaleCurrentImage((double)w / ((double)(image->width())),
                            (double)h / ((double)(image->height())),
                            dlgImageSize->filterType());
        }
        else {
            m_view->resizeCurrentImage(w, h, dlgImageSize->cropLayers());
        }
    }

    delete dlgImageSize;
}

void ImageSize::slotLayerSize()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgLayerSize * dlgLayerSize = new DlgLayerSize(m_view, "LayerSize");
    Q_CHECK_PTR(dlgLayerSize);

    dlgLayerSize->setCaption(i18n("Layer Size"));

    KisConfig cfg;
    KisPaintDeviceSP dev = image->activeDevice();

    QRect rc = dev->exactBounds();

    dlgLayerSize->setWidth(rc.width());
    dlgLayerSize->setHeight(rc.height());

    if (dlgLayerSize->exec() == QDialog::Accepted) {
        Q_INT32 w = dlgLayerSize->width();
        Q_INT32 h = dlgLayerSize->height();

        m_view->scaleLayer((double)w / ((double)(rc.width())),
                           (double)h / ((double)(rc.height())),
                           dlgLayerSize->filterType());
    }
    delete dlgLayerSize;
}

void ImageSize::slotSelectionScale()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    KisPaintDeviceSP layer = image->activeDevice();

    if (!layer) return;

    if (!layer->hasSelection()) return;


    DlgLayerSize * dlgLayerSize = new DlgLayerSize(m_view, "SelectionScale");
    Q_CHECK_PTR(dlgLayerSize);

    dlgLayerSize->setCaption(i18n("Scale Selection"));

    KisConfig cfg;
    QRect rc = layer->selection()->selectedRect();

    dlgLayerSize->setWidth(rc.width());
    dlgLayerSize->setHeight(rc.height());

    if (dlgLayerSize->exec() == QDialog::Accepted) {
        Q_INT32 w = dlgLayerSize->width();
        Q_INT32 h = dlgLayerSize->height();

        KisScaleWorker worker (layer->selection().data(),
                               (double)w / ((double)(rc.width())),
                               (double)h / ((double)(rc.height())),
                               dlgLayerSize->filterType());
        worker.run();

        m_view->getCanvasController()->updateCanvas();

    }
    delete dlgLayerSize;
}


#include "imagesize.moc"
