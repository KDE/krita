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

#include "imagesize.h"


#include <math.h>

#include <stdlib.h>

#include <QSlider>
#include <QPoint>
#include <QRect>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_statusbar.h>
#include <kis_types.h>
#include <kis_view2.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_transaction.h>
#include <kis_image_manager.h>
#include <kis_layer_manager.h>
#include <kis_transform_visitor.h>
#include <widgets/kis_progress_widget.h>

#include "dlg_imagesize.h"
#include "dlg_canvassize.h"
#include "dlg_layersize.h"
#include "kis_filter_strategy.h"
#include "kis_canvas_resource_provider.h"

typedef KGenericFactory<ImageSize> ImageSizeFactory;
K_EXPORT_COMPONENT_FACTORY(kritaimagesize, ImageSizeFactory("krita"))

ImageSize::ImageSize(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        setComponentData(ImageSizeFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/imagesize.rc"), true);

        KAction *action  = new KAction(i18n("Scale To New Size..."), this);
        actionCollection()->addAction("imagesize", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotImageSize()));

        action = new KAction(i18n("Size Canvas..."), this);
        actionCollection()->addAction("canvassize", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotCanvasSize()));

        m_scaleLayerAction = new KAction(i18n("Scale &Layer..."), this);
        actionCollection()->addAction("layersize", m_scaleLayerAction);
        connect(m_scaleLayerAction, SIGNAL(triggered()), this, SLOT(slotLayerSize()));

        m_view = (KisView2*) parent;
        // Selection manager takes ownership?
        action  = new KAction(i18n("&Scale Selection..."), this);
        actionCollection()->addAction("selectionscale", action);
        Q_CHECK_PTR(action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotSelectionScale()));

        m_view ->selectionManager()->addSelectionAction(action);
        connect(m_view->resourceProvider(), SIGNAL(sigNodeChanged(const KisNodeSP)), SLOT(slotNodeChanged(KisNodeSP)));
    }
}

ImageSize::~ImageSize()
{
    m_view = 0;
}

void ImageSize::slotImageSize()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    KisConfig cfg;

    DlgImageSize * dlgImageSize = new DlgImageSize(m_view, image->width(), image->height(), image->yRes());
    dlgImageSize->setObjectName("ImageSize");
    Q_CHECK_PTR(dlgImageSize);

    if (dlgImageSize->exec() == QDialog::Accepted) {
        qint32 w = dlgImageSize->width();
        qint32 h = dlgImageSize->height();
        double res = dlgImageSize->resolution();

        image->setResolution(res, res);

        if (w != image->width() || h != image->height())
            m_view->imageManager()->scaleCurrentImage((double)w / ((double)(image->width())),
                    (double)h / ((double)(image->height())), dlgImageSize->filterType());
        image->rootLayer()->setDirty();
    }

    delete dlgImageSize;
}

void ImageSize::slotCanvasSize()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgCanvasSize * dlgCanvasSize = new DlgCanvasSize(m_view, image->width(), image->height());
    Q_CHECK_PTR(dlgCanvasSize);

    if (dlgCanvasSize->exec() == QDialog::Accepted) {
        qint32 width = dlgCanvasSize->width();
        qint32 height = dlgCanvasSize->height();
        qint32 xOffset = dlgCanvasSize->xOffset();
        qint32 yOffset = dlgCanvasSize->yOffset();

        m_view->imageManager()->resizeCurrentImage(width, height, xOffset, yOffset);
    }
    delete dlgCanvasSize;
}

void ImageSize::slotLayerSize()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgLayerSize * dlgLayerSize = new DlgLayerSize(m_view, "LayerSize");
    Q_CHECK_PTR(dlgLayerSize);

    dlgLayerSize->setCaption(i18n("Layer Size"));

    KisConfig cfg;

    KisPaintDeviceSP dev = m_view->activeDevice();
    Q_ASSERT(dev);
    QRect rc = dev->exactBounds();

    dlgLayerSize->setWidth(rc.width());
    dlgLayerSize->setHeight(rc.height());

    if (dlgLayerSize->exec() == QDialog::Accepted) {
        qint32 w = dlgLayerSize->width();
        qint32 h = dlgLayerSize->height();

        m_view->layerManager()->scaleLayer((double)w / ((double)(rc.width())),
                                           (double)h / ((double)(rc.height())),
                                           dlgLayerSize->filterType());
    }
    delete dlgLayerSize;
}

void ImageSize::slotSelectionScale()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    KisPaintDeviceSP layer = m_view->activeDevice();

    if (!layer) return;

    KisSelectionSP selection = m_view->selection();
    if (!selection) return;


    DlgLayerSize * dlgSize = new DlgLayerSize(m_view, "SelectionScale");
    Q_CHECK_PTR(dlgSize);

    dlgSize->setCaption(i18n("Scale Selection"));

    KisConfig cfg;

    QRect rc = selection->selectedRect();

    dlgSize->setWidth(rc.width());
    dlgSize->setHeight(rc.height());


    KoProgressUpdater* pu = m_view->createProgressUpdater();
    QPointer<KoUpdater> u = pu->startSubtask();

    if (dlgSize->exec() == QDialog::Accepted) {
        qint32 w = dlgSize->width();
        qint32 h = dlgSize->height();
        KisTransformWorker worker(selection.data(),
                                  (double)w / ((double)(rc.width())),
                                  (double)h / ((double)(rc.height())),
                                  0, 0, 0.0, 0, 0, u,
                                  dlgSize->filterType()
                                 );
        worker.run();
        layer->setDirty();
        pu->deleteLater();
    }
    delete dlgSize;
}


void ImageSize::slotNodeChanged(const KisNodeSP node)
{
    m_scaleLayerAction->setEnabled(m_view->activeDevice());
}

#include "imagesize.moc"
