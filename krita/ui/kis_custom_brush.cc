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

#include <KoImageResource.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <QPixmap>
#include <QShowEvent>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include "kis_view.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_custom_brush.h"
#include "kis_resource_mediator.h"
#include "kis_resourceserver.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"

KisCustomBrush::KisCustomBrush(QWidget *parent, const char* name, const QString& caption, KisView* view)
    : KisWdgCustomBrush(parent, name), m_view(view)
{
    Q_ASSERT(m_view);
    m_mediator = 0;
    setCaption(caption);

    m_brush = 0;

    preview->setScaledContents(true);

    connect(addButton, SIGNAL(pressed()), this, SLOT(slotAddPredefined()));
    connect(brushButton, SIGNAL(pressed()), this, SLOT(slotUseBrush()));
//    connect(exportButton, SIGNAL(pressed()), this, SLOT(slotExport()));
    connect(style, SIGNAL(activated(int)), this, SLOT(slotUpdateCurrentBrush(int)));
    connect(colorAsMask, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateCurrentBrush(int)));
}

KisCustomBrush::~KisCustomBrush() {
    delete m_brush;
}

void KisCustomBrush::showEvent(QShowEvent *) {
    slotUpdateCurrentBrush(0);
}

void KisCustomBrush::slotUpdateCurrentBrush(int) {
    delete m_brush;
    if (m_view->canvasSubject() && m_view->canvasSubject()->currentImg()) {
        createBrush();
        preview->setPixmap(QPixmap(m_brush->img()));
    } else {
        m_brush = 0;
    }
}

void KisCustomBrush::slotExport() {
    ;
}

void KisCustomBrush::slotAddPredefined() {
    // Save in the directory that is likely to be: ~/.kde/share/apps/krita/brushes
    // a unique file with this brushname
    QString dir = KGlobal::dirs()->saveLocation("data", "krita/brushes");
    QString extension;

    if (style->currentItem() == 0) {
        extension = ".gbr";
    } else {
        extension = ".gih";
    }
    KTempFile file(dir, extension);
    file.close(); // If we don't, and brush->save first, it might get truncated!

    // Save it to that file 
    m_brush->setFilename(file.name());

    // Add it to the brush server, so that it automatically gets to the mediators, and
    // so to the other brush choosers can pick it up, if they want to
    if (m_server)
        m_server->addResource(m_brush->clone());
}

void KisCustomBrush::slotUseBrush() {
    KisBrush* copy = m_brush->clone();

    Q_CHECK_PTR(copy);

    emit(activatedResource(copy));
}

void KisCustomBrush::createBrush() {
    KisImageSP img = m_view->canvasSubject()->currentImg();

    if (!img)
        return;

    if (style->currentItem() == 0) {
        m_brush = new KisBrush(img->mergedImage(), 0, 0, img->width(), img->height());
        if (colorAsMask->isChecked())
            m_brush->makeMaskImage();
        return;
    }

    // For each layer in the current image, create a new image, and add it to the list
    Q3ValueVector< Q3ValueVector<KisPaintDevice*> > devices;
    devices.push_back(Q3ValueVector<KisPaintDevice*>());
    int w = img->width();
    int h = img->height();

    // We only loop over the rootLayer. Since we actually should have a layer selection
    // list, no need to elaborate on that here and now
    KisLayer* layer = img->rootLayer()->firstChild();
    while (layer) {
        KisPaintLayer* paint = 0;
        if (layer->visible() && (paint = dynamic_cast<KisPaintLayer*>(layer)))
            devices.at(0).push_back(paint->paintDevice());
        layer = layer->nextSibling();
    }
    Q3ValueVector<KisPipeBrushParasite::SelectionMode> modes;

    switch(comboBox2->currentItem()) {
        case 0: modes.push_back(KisPipeBrushParasite::Constant); break;
        case 1: modes.push_back(KisPipeBrushParasite::Random); break;
        case 2: modes.push_back(KisPipeBrushParasite::Incremental); break;
        case 3: modes.push_back(KisPipeBrushParasite::Pressure); break;
        case 4: modes.push_back(KisPipeBrushParasite::Angular); break;
        default: modes.push_back(KisPipeBrushParasite::Incremental);
    }

    m_brush = new KisImagePipeBrush(img->name(), w, h, devices, modes);
    if (colorAsMask->isChecked())
        m_brush->makeMaskImage();
}


#include "kis_custom_brush.moc"
