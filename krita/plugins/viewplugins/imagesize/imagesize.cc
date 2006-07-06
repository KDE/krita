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

#include <QSlider>
#include <QPoint>

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

#include "imagesize.h"
#include "dlg_imagesize.h"
#include "dlg_layersize.h"
#include "kis_filter_strategy.h"

typedef KGenericFactory<ImageSize> ImageSizeFactory;
K_EXPORT_COMPONENT_FACTORY( kritaimagesize, ImageSizeFactory( "krita" ) )

ImageSize::ImageSize(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView") )
    {
        setInstance(ImageSizeFactory::instance());
        
setXMLFile(KStandardDirs::locate("data","kritaplugins/imagesize.rc"), 
true);

        KAction *action = new KAction(i18n("Change &Image Size..."), actionCollection(), "imagesize");
        action->setShortcut(Qt::SHIFT+Qt::Key_S);
        connect(action, SIGNAL(triggered()), this, SLOT(slotImageSize()));

        action = new KAction(i18n("Change &Layer Size..."), actionCollection(), "layersize");
        connect(action, SIGNAL(triggered()), this, SLOT(slotLayerSize()));

        m_view = (KisView*) parent;
        // Selection manager takes ownership?
        action = new KAction(i18n("&Layer Size..."), actionCollection(), "selectionScale");
        Q_CHECK_PTR(action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotLayerSize()));

        m_view ->canvasSubject()-> selectionManager()->addSelectionAction(action);
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
        qint32 w = dlgImageSize->width();
        qint32 h = dlgImageSize->height();

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

    dlgLayerSize->setWidth(image->width());
    dlgLayerSize->setHeight(image->height());

    if (dlgLayerSize->exec() == QDialog::Accepted) {
        qint32 w = dlgLayerSize->width();
        qint32 h = dlgLayerSize->height();

        m_view->scaleLayer((double)w / ((double)(image->width())),
                    (double)h / ((double)(image->height())),
                    dlgLayerSize->filterType());
    }
    delete dlgLayerSize;
}

void ImageSize::slotSelectionScale()
{
    // XXX: figure out a way to add selection actions to the selection
    // manager to enable/disable

    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    KisPaintDeviceSP layer = image->activeDevice();

    if (!layer) return;

    if (!layer->hasSelection()) return;


    DlgImageSize * dlgImageSize = new DlgImageSize(m_view, "SelectionScale");
    Q_CHECK_PTR(dlgImageSize);

    dlgImageSize->setCaption(i18n("Scale Selection"));

    KisConfig cfg;

    dlgImageSize->setWidth(image->width());
    dlgImageSize->setHeight(image->height());

    dlgImageSize->hideScaleBox();

    if (dlgImageSize->exec() == QDialog::Accepted) {
        qint32 w = dlgImageSize->width();
        qint32 h = dlgImageSize->height();

        m_view->scaleLayer((double)w / ((double)(image->width())),
                     (double)h / ((double)(image->height())),
                     dlgImageSize->filterType());

    }
    delete dlgImageSize;
}


#include "imagesize.moc"
