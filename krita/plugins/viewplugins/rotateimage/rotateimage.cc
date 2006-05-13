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
        setXMLFile(locate("data","kritaplugins/rotateimage.rc"), true);
        m_view = (KisView*) parent;
        (void) new KAction(i18n("&Rotate Image..."), 0, 0, this, SLOT(slotRotateImage()), actionCollection(), "rotateimage");
        (void) new KAction(i18n("Rotate Image CW"), "rotate_cw", 0, this, SLOT(slotRotateImage90()), actionCollection(), "rotateImageCW90");
        (void) new KAction(i18n("Rotate Image 1&80"), 0, 0, this, SLOT(slotRotateImage180()), actionCollection(), "rotateImage180");
        (void) new KAction(i18n("Rotate Image CCW"), "rotate_ccw", 0, this, SLOT(slotRotateImage270()), actionCollection(), "rotateImageCCW90");

        (void) new KAction(i18n("&Rotate Layer..."), 0, 0, this, SLOT(slotRotateLayer()), actionCollection(), "rotatelayer");

        (void)new KAction(i18n("Rotate 1&80"), 0, m_view, SLOT(rotateLayer180()), actionCollection(), "rotateLayer180");
        (void)new KAction(i18n("Rotate CCW"), "rotate_ccw", 0, m_view, SLOT(rotateLayerLeft90()), actionCollection(), "rotateLayerCCW90");
        (void)new KAction(i18n("Rotate CW"), "rotate_cw", 0, m_view, SLOT(rotateLayerRight90()), actionCollection(), "rotateLayerCW90");
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
