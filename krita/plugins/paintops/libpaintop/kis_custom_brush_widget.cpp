/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <QDateTime>

#include <QPixmap>
#include <QShowEvent>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_gbr_brush.h"
#include "kis_imagepipe_brush.h"

#include "kis_brush_server.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include <kis_selection.h>
#include <KoProperties.h>
#include "kis_iterator_ng.h"

KisCustomBrushWidget::KisCustomBrushWidget(QWidget *parent, const QString& caption, KisImageWSP image)
        : KisWdgCustomBrush(parent)
        , m_image(image)
{
    setWindowTitle(caption);
    preview->setScaledContents(true);
    preview->setFixedSize(preview->size());

    KoResourceServer<KisBrush>* rServer = KisBrushServer::instance()->brushServer();
    m_rServerAdapter = new KoResourceServerAdapter<KisBrush>(rServer);

    m_brush = 0;
    m_brushCreated = false;

    connect(addButton, SIGNAL(pressed()), this, SLOT(slotAddPredefined()));
    connect(brushButton, SIGNAL(pressed()), this, SLOT(slotUpdateCurrentBrush()));
    connect(brushStyle, SIGNAL(activated(int)), this, SLOT(slotUpdateCurrentBrush(int)));
    connect(colorAsMask, SIGNAL(toggled(bool)), this, SLOT(slotUpdateUseColorAsMask(bool)));
    connect(spacingSlider, SIGNAL(valueChanged(qreal)), this, SLOT(slotUpdateSpacing(qreal)));
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
    if (!m_brushCreated){
        slotUpdateCurrentBrush(0);
        m_brushCreated = true;
    }
}

void KisCustomBrushWidget::slotUpdateCurrentBrush(int)
{
    if (brushStyle->currentIndex() == 0) {
        comboBox2->setEnabled(false);
    }
    else {
        comboBox2->setEnabled(true);
    }
    if (m_image) {
        createBrush();
        if (m_brush){
            preview->setPixmap(QPixmap::fromImage( m_brush->image() ));
        }
    }
    emit sigBrushChanged();
}

void KisCustomBrushWidget::slotUpdateSpacing(qreal spacing)
{
    if (m_brush) {
        m_brush->setSpacing(spacing);
    }
    emit sigBrushChanged();
}

void KisCustomBrushWidget::slotUpdateUseColorAsMask(bool useColorAsMask)
{
    if (m_brush){
        static_cast<KisGbrBrush*>( m_brush.data() )->setUseColorAsMask( useColorAsMask );
        preview->setPixmap(QPixmap::fromImage( m_brush->image() ));
    }
    emit sigBrushChanged();
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

    QString name = nameLineEdit->text();
    QString tempFileName;
    {
        QFileInfo fileInfo;
        fileInfo.setFile(dir + name + extension);

        int i = 1;
        while (fileInfo.exists()) {
            fileInfo.setFile(dir + name + QString("%1").arg(i) + extension);
            i++;
        }

        tempFileName = fileInfo.filePath();
    }

    // Add it to the brush server, so that it automatically gets to the mediators, and
    // so to the other brush choosers can pick it up, if they want to
    if (m_rServerAdapter) {
        KisGbrBrush * resource = static_cast<KisGbrBrush*>( m_brush.data() )->clone();
        resource->setFilename(tempFileName);

        if (nameLineEdit->text().isEmpty()){
            resource->setName(QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm"));
        }else{
            resource->setName( name );
        }

        if (colorAsMask->isChecked()){
            resource->makeMaskImage();
        }

        m_rServerAdapter->addResource( resource );
    }
}

void KisCustomBrushWidget::createBrush()
{
    if (!m_image)
        return;

    if (m_brush){
        // don't delete shared pointer, please
        bool removedCorrectly = KisBrushServer::instance()->brushServer()->removeResourceFromServer(  m_brush.data() );
        if (!removedCorrectly){
            kWarning() << "Brush was not removed correctly for the resource server";
        }
    }

    if (brushStyle->currentIndex() == 0) {
        KisSelectionSP selection = m_image->globalSelection();
        // create copy of the data
        m_image->lock();
        KisPaintDeviceSP dev = new KisPaintDevice(*m_image->mergedImage());
        m_image->unlock();

        if (!selection){
            m_brush = new KisGbrBrush(dev, 0, 0, m_image->width(), m_image->height());
        }
        else {
            // apply selection mask
            QRect r = selection->selectedExactRect();
            dev->crop(r);

            KisHLineIteratorSP pixelIt = dev->createHLineIteratorNG(r.x(), r.top(), r.width());
            KisHLineConstIteratorSP maskIt = selection->projection()->createHLineIteratorNG(r.x(), r.top(), r.width());

            for (qint32 y = r.top(); y <= r.bottom(); ++y) {

                do {
                    dev->colorSpace()->applyAlphaU8Mask(pixelIt->rawData(), maskIt->oldRawData(), 1);
                } while (pixelIt->nextPixel() && maskIt->nextPixel());

                pixelIt->nextRow();
                maskIt->nextRow();
            }

            QRect rc = dev->exactBounds();
            m_brush = new KisGbrBrush(dev, rc.x(), rc.y(), rc.width(), rc.height());
        }

    } else {
        // For each layer in the current image, create a new image, and add it to the list
        QVector< QVector<KisPaintDevice*> > devices;
        devices.push_back(QVector<KisPaintDevice*>());
        int w = m_image->width();
        int h = m_image->height();

        m_image->lock();

        // We only loop over the rootLayer. Since we actually should have a layer selection
        // list, no need to elaborate on that here and now
        KoProperties properties;
        properties.setProperty("visible", true);
        QList<KisNodeSP> layers = m_image->root()->childNodes(QStringList("KisLayer"), properties);
        KisNodeSP node;
        foreach(KisNodeSP node, layers) {
            devices[0].push_back(node->projection().data());
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
        m_image->unlock();
    }

    static_cast<KisGbrBrush*>( m_brush.data() )->setUseColorAsMask( colorAsMask->isChecked() );
    m_brush->setSpacing(spacingSlider->value());
    m_brush->setFilename(TEMPORARY_FILENAME);
    m_brush->setName(TEMPORARY_BRUSH_NAME);
    m_brush->setValid(true);

    KisBrushServer::instance()->brushServer()->addResource( m_brush.data() , false);
}

void KisCustomBrushWidget::setImage(KisImageWSP image)
{
    m_image = image;
}


#include "kis_custom_brush_widget.moc"
