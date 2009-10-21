/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_custom_brush_widget.h"

#include <KoImageResource.h>
#include <kis_debug.h>
#include <QLabel>
#include <QImage>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>

#include <QPixmap>
#include <QShowEvent>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_gbr_brush.h"
#include "kis_imagepipe_brush.h"

#include "kis_resource_mediator.h"
#include "kis_brush_server.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"

KisCustomBrushWidget::KisCustomBrushWidget(QWidget *parent, const QString& caption, KisImageWSP image)
        : KisWdgCustomBrush(parent)
        , m_image(image)
{
    setWindowTitle(caption);
    preview->setScaledContents(true);
    preview->setFixedSize(preview->size());

    KoResourceServer<KisBrush>* rServer = KisBrushServer::instance()->brushServer();
    m_rServerAdapter = new KoResourceServerAdapter<KisBrush>(rServer);

    connect(addButton, SIGNAL(pressed()), this, SLOT(slotAddPredefined()));
    connect(brushButton, SIGNAL(pressed()), this, SLOT(slotUpdateCurrentBrush()));
    //    connect(exportButton, SIGNAL(pressed()), this, SLOT(slotExport()));
    connect(brushStyle, SIGNAL(activated(int)), this, SLOT(slotUpdateCurrentBrush(int)));
    connect(colorAsMask, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateCurrentBrush(int)));
    slotUpdateCurrentBrush();
}

KisCustomBrushWidget::~KisCustomBrushWidget()
{
    delete m_rServerAdapter;
}


KisBrushSP KisCustomBrushWidget::brush()
{
    return m_brush;
}

void KisCustomBrushWidget::showEvent(QShowEvent *)
{
    slotUpdateCurrentBrush(0);
}

void KisCustomBrushWidget::slotUpdateCurrentBrush(int)
{
    if (m_image) {
        createBrush();
        if (m_brush)
            preview->setPixmap(QPixmap::fromImage(m_brush->img()));
    }
    emit sigBrushChanged();
}

void KisCustomBrushWidget::slotExport()
{
    ;
}

void KisCustomBrushWidget::slotAddPredefined()
{
    // Save in the directory that is likely to be: ~/.kde/share/apps/krita/brushes
    // a unique file with this brushname
    QString dir = KGlobal::dirs()->saveLocation("data", "krita/brushes");
    QString extension;

    if (brushStyle->currentIndex() == 0) {
        extension = ".gbr";
    } else {
        extension = ".gih";
    }

    QString tempFileName;
    {
        KTemporaryFile file;
        file.setPrefix(dir);
        file.setSuffix(extension);
        file.setAutoRemove(false);
        file.open();
        tempFileName = file.fileName();
    }

    // Save it to that file
    m_brush->setFilename(tempFileName);
    m_brush->setValid(true);

    // Add it to the brush server, so that it automatically gets to the mediators, and
    // so to the other brush choosers can pick it up, if they want to
    if (m_rServerAdapter)
        m_rServerAdapter->addResource(static_cast<KisGbrBrush*>(m_brush.data())->clone());
}

void KisCustomBrushWidget::createBrush()
{
    if (!m_image)
        return;

    if (brushStyle->currentIndex() == 0) {
        m_brush = new KisGbrBrush(m_image->mergedImage().data(), 0, 0, m_image->width(), m_image->height());
        if (colorAsMask->isChecked())
            static_cast<KisGbrBrush*>(m_brush.data())->makeMaskImage();
        return;
    }

    // For each layer in the current image, create a new image, and add it to the list
    QVector< QVector<KisPaintDevice*> > devices;
    devices.push_back(QVector<KisPaintDevice*>());
    int w = m_image->width();
    int h = m_image->height();

    // We only loop over the rootLayer. Since we actually should have a layer selection
    // list, no need to elaborate on that here and now
    KisLayer* layer = dynamic_cast<KisLayer*>(m_image->rootLayer()->firstChild().data());
    while (layer) {
        KisPaintLayer* paint = 0;
        if (layer->visible() && (paint = dynamic_cast<KisPaintLayer*>(layer)))
            devices[0].push_back(paint->paintDevice().data());
        layer = dynamic_cast<KisLayer*>(layer->nextSibling().data());
    }
    QVector<KisParasite::SelectionMode> modes;

    switch (comboBox2->currentIndex()) {
    case 0: modes.push_back(KisParasite::Constant); break;
    case 1: modes.push_back(KisParasite::Random); break;
    case 2: modes.push_back(KisParasite::Incremental); break;
    case 3: modes.push_back(KisParasite::Pressure); break;
    case 4: modes.push_back(KisParasite::Angular); break;
    default: modes.push_back(KisParasite::Incremental);
    }

    m_brush = new KisImagePipeBrush(m_image->objectName(), w, h, devices, modes);
    if (colorAsMask->isChecked())
        static_cast<KisGbrBrush*>(m_brush.data())->makeMaskImage();
}

void KisCustomBrushWidget::setImage(KisImageWSP image)
{
    m_image = image;
}


#include "kis_custom_brush_widget.moc"
