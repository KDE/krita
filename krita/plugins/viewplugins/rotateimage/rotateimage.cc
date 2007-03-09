/*
 * rotateimage.cc -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler
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
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kactioncollection.h>
#include <kicon.h>
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view2.h>
#include <kis_selection.h>
#include <kis_image_manager.h>
#include <kis_layer_manager.h>

#include "rotateimage.h"
#include "dlg_rotateimage.h"

typedef KGenericFactory<RotateImage> RotateImageFactory;
K_EXPORT_COMPONENT_FACTORY( kritarotateimage, RotateImageFactory( "krita" ) )

// XXX: this plugin could also provide layer scaling/resizing
RotateImage::RotateImage(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView2") ) {
        setComponentData(RotateImageFactory::componentData());

setXMLFile(KStandardDirs::locate("data","kritaplugins/rotateimage.rc"),
true);
        m_view = (KisView2*) parent;

        KAction *action  = new KAction(i18n("&Rotate Image..."), this);
        actionCollection()->addAction("rotateimage", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage()));

        action  = new KAction(KIcon("object-rotate-left"), i18n("Rotate Image CW"), this);
        actionCollection()->addAction("rotateImageCW90", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage90()));

        action  = new KAction(i18n("Rotate Image 1&80"), this);
        actionCollection()->addAction("rotateImage180", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage180()));

        action  = new KAction(KIcon("object-rotate-right"), i18n("Rotate Image CCW"), this);
        actionCollection()->addAction("rotateImageCCW90", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage270()));

        action  = new KAction(i18n("&Rotate Layer..."), this);
        actionCollection()->addAction("rotatelayer", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateLayer()));

        action  = new KAction(i18n("Rotate 1&80"), this);
        actionCollection()->addAction("rotateLayer180", action );
        connect(action, SIGNAL(triggered()), m_view->layerManager(), SLOT(rotateLayer180()));

        action  = new KAction(KIcon("object-rotate-right"), i18n("Rotate CCW"), this);
        actionCollection()->addAction("rotateLayerCCW90", action );
        connect(action, SIGNAL(triggered()), m_view->layerManager(), SLOT(rotateLayerLeft90()));

        action  = new KAction(KIcon("object-rotate-left"), i18n("Rotate CW"), this);
        actionCollection()->addAction("rotateLayerCW90", action );
        connect(action, SIGNAL(triggered()), m_view->layerManager(), SLOT(rotateLayerRight90()));
    }
}

RotateImage::~RotateImage()
{
    m_view = 0;
}

void RotateImage::slotRotateImage()
{
    KisImageSP image = m_view->image();

    if (!image) return;

    DlgRotateImage * dlgRotateImage = new DlgRotateImage(m_view, "RotateImage");
    Q_CHECK_PTR(dlgRotateImage);

    dlgRotateImage->setCaption(i18n("Rotate Image"));

        if (dlgRotateImage->exec() == QDialog::Accepted) {
        double angle = dlgRotateImage->angle() * M_PI/180;
        m_view->imageManager()->rotateCurrentImage(angle);
    }
    delete dlgRotateImage;
}

void RotateImage::slotRotateImage90()
{
    m_view->imageManager()->rotateCurrentImage( M_PI/2 );
}

void RotateImage::slotRotateImage180()
{
    m_view->imageManager()->rotateCurrentImage( M_PI );
}


void RotateImage::slotRotateImage270()
{
    m_view->imageManager()->rotateCurrentImage( - M_PI/2 + M_PI*2 );
}

void RotateImage::slotRotateLayer()
{
    KisImageSP image = m_view->image();

    if (!image) return;

    DlgRotateImage * dlgRotateImage = new DlgRotateImage(m_view, "RotateLayer");
    Q_CHECK_PTR(dlgRotateImage);

    dlgRotateImage->setCaption(i18n("Rotate Layer"));

    if (dlgRotateImage->exec() == QDialog::Accepted) {
        double angle = dlgRotateImage->angle() * M_PI/180;
        m_view->layerManager()->rotateLayer(angle);

    }
    delete dlgRotateImage;
}

#include "rotateimage.moc"
