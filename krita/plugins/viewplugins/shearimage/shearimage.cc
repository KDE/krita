/*
 * shearimage.cc -- Part of Krita
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

#include <qslider.h>
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

#include "shearimage.h"
#include "dlg_shearimage.h"

typedef KGenericFactory<ShearImage> ShearImageFactory;
K_EXPORT_COMPONENT_FACTORY( kritashearimage, ShearImageFactory( "krita" ) )

// XXX: this plugin could also provide layer scaling/resizing
ShearImage::ShearImage(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView") )
    {
        setInstance(ShearImageFactory::instance());
        setXMLFile(locate("data","kritaplugins/shearimage.rc"), true);

        (void) new KAction(i18n("&Shear Image..."), 0, 0, this, SLOT(slotShearImage()), actionCollection(), "shearimage");
        (void) new KAction(i18n("&Shear Layer..."), 0, 0, this, SLOT(slotShearLayer()), actionCollection(), "shearlayer");

        m_view = (KisView*) parent;
    }
}

ShearImage::~ShearImage()
{
    m_view = 0;
}

void ShearImage::slotShearImage()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgShearImage * dlgShearImage = new DlgShearImage(m_view, "ShearImage");
    Q_CHECK_PTR(dlgShearImage);

    dlgShearImage->setCaption(i18n("Shear Image"));

        if (dlgShearImage->exec() == QDialog::Accepted) {
        qint32 angleX = dlgShearImage->angleX();
                qint32 angleY = dlgShearImage->angleY();
                m_view->shearCurrentImage(angleX, angleY);
    }
        delete dlgShearImage;
}

void ShearImage::slotShearLayer()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgShearImage * dlgShearImage = new DlgShearImage(m_view, "ShearLayer");
    Q_CHECK_PTR(dlgShearImage);

    dlgShearImage->setCaption(i18n("Shear Layer"));

    if (dlgShearImage->exec() == QDialog::Accepted) {
                qint32 angleX = dlgShearImage->angleX();
                qint32 angleY = dlgShearImage->angleY();
                m_view->shearLayer(angleX, angleY);

    }
    delete dlgShearImage;
}

#include "shearimage.moc"
