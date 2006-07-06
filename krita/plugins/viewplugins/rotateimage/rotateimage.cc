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
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>

#include "rotateimage.h"
#include "dlg_rotateimage.h"

typedef KGenericFactory<RotateImage> RotateImageFactory;
K_EXPORT_COMPONENT_FACTORY( kritarotateimage, RotateImageFactory( "krita" ) )

// XXX: this plugin could also provide layer scaling/resizing
RotateImage::RotateImage(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView") ) {
        setInstance(RotateImageFactory::instance());
        
setXMLFile(KStandardDirs::locate("data","kritaplugins/rotateimage.rc"), 
true);
        m_view = (KisView*) parent;

        KAction *action = new KAction(i18n("&Rotate Image..."), actionCollection(), "rotateimage");
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage()));

        action = new KAction(KIcon("rotate_cw"), i18n("Rotate Image CW"), actionCollection(), "rotateImageCW90");
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage90()));

        action = new KAction(i18n("Rotate Image 1&80"), actionCollection(), "rotateImage180");
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage180()));

        action = new KAction(KIcon("rotate_ccw"), i18n("Rotate Image CCW"), actionCollection(), "rotateImageCCW90");
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage270()));

        action = new KAction(i18n("&Rotate Layer..."), actionCollection(), "rotatelayer");
        connect(action, SIGNAL(triggered()), this, SLOT(slotRotateLayer()));

        action = new KAction(i18n("Rotate 1&80"), actionCollection(), "rotateLayer180");
        connect(action, SIGNAL(triggered()), m_view, SLOT(rotateLayer180()));

        action = new KAction(KIcon("rotate_ccw"), i18n("Rotate CCW"), actionCollection(), "rotateLayerCCW90");
        connect(action, SIGNAL(triggered()), m_view, SLOT(rotateLayerLeft90()));

        action = new KAction(KIcon("rotate_cw"), i18n("Rotate CW"), actionCollection(), "rotateLayerCW90");
        connect(action, SIGNAL(triggered()), m_view, SLOT(rotateLayerRight90()));
    }
}

RotateImage::~RotateImage()
{
    m_view = 0;
}

void RotateImage::slotRotateImage()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgRotateImage * dlgRotateImage = new DlgRotateImage(m_view, "RotateImage");
    Q_CHECK_PTR(dlgRotateImage);

    dlgRotateImage->setCaption(i18n("Rotate Image"));

        if (dlgRotateImage->exec() == QDialog::Accepted) {
        qint32 angle = dlgRotateImage->angle();
        m_view->rotateCurrentImage(angle);
    }
    delete dlgRotateImage;
}

void RotateImage::slotRotateImage90()
{
    m_view->rotateCurrentImage( 90 );
}

void RotateImage::slotRotateImage180()
{
    m_view->rotateCurrentImage( 180 );
}


void RotateImage::slotRotateImage270()
{
    m_view->rotateCurrentImage( 270 );
}

void RotateImage::slotRotateLayer()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgRotateImage * dlgRotateImage = new DlgRotateImage(m_view, "RotateLayer");
    Q_CHECK_PTR(dlgRotateImage);

    dlgRotateImage->setCaption(i18n("Rotate Layer"));

    if (dlgRotateImage->exec() == QDialog::Accepted) {
                qint32 angle = dlgRotateImage->angle();
        m_view->rotateLayer(angle);

    }
    delete dlgRotateImage;
}

#include "rotateimage.moc"
