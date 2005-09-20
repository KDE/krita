/*
 * colorspaceconversion.cc -- Part of Krita
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
#include <stdlib.h>

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qapplication.h>
#include <qcursor.h>

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
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_paint_device_impl.h>
#include <kis_colorspace_registry.h>
#include "colorspaceconversion.h"
#include "dlg_colorspaceconversion.h"

typedef KGenericFactory<ColorSpaceConversion> ColorSpaceConversionFactory;
K_EXPORT_COMPONENT_FACTORY( kritacolorspaceconversion, ColorSpaceConversionFactory( "krita" ) )


ColorSpaceConversion::ColorSpaceConversion(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{

    setInstance(ColorSpaceConversionFactory::instance());

     kdDebug(DBG_AREA_PLUGINS) << "ColorSpaceconversion plugin. Class: "
           << className()
           << ", Parent: "
           << parent -> className()
           << "\n";

    (void) new KAction(i18n("&Convert Image Type..."), 0, 0, this, SLOT(slotImgColorSpaceConversion()), actionCollection(), "imgcolorspaceconversion");
    (void) new KAction(i18n("&Convert Layer Type..."), 0, 0, this, SLOT(slotLayerColorSpaceConversion()), actionCollection(), "layercolorspaceconversion");

    if ( !parent->inherits("KisView") )
    {
        m_view = 0;
    } else {
        m_view = (KisView*) parent;
    }
}

ColorSpaceConversion::~ColorSpaceConversion()
{
    m_view = 0;
}

void ColorSpaceConversion::slotImgColorSpaceConversion()
{
    KisImageSP image = m_view -> getCanvasSubject() -> currentImg();

    if (!image) return;

    DlgColorSpaceConversion * dlgColorSpaceConversion = new DlgColorSpaceConversion(m_view, "ColorSpaceConversion");
    Q_CHECK_PTR(dlgColorSpaceConversion);

    dlgColorSpaceConversion -> setCaption(i18n("Convert All Layers From ") + image -> colorSpace() -> id().name());

    dlgColorSpaceConversion -> fillCmbSrcProfile(image -> colorSpace() -> id());

    if (image -> profile()) {
        dlgColorSpaceConversion -> m_page -> cmbSourceProfile -> setCurrentText(image -> profile() -> productName());
    }

    if (dlgColorSpaceConversion -> exec() == QDialog::Accepted) {
        // XXX: Do the rest of the stuff
        KisID cspace = dlgColorSpaceConversion -> m_page -> cmbColorSpaces -> currentItem();
        KisColorSpace * cs = KisColorSpaceRegistry::instance() -> get(cspace);
        // XXX: Should we actually set the profile here?
        image -> setProfile(KisColorSpaceRegistry::instance()->getProfileByName(dlgColorSpaceConversion -> m_page -> cmbSourceProfile -> currentText()));
        QApplication::setOverrideCursor(KisCursor::waitCursor());
        image -> convertTo(cs,
                           KisColorSpaceRegistry::instance()->getProfileByName(dlgColorSpaceConversion -> m_page -> cmbDestProfile -> currentText()),
                           dlgColorSpaceConversion -> m_page -> grpIntent -> selectedId());
        QApplication::restoreOverrideCursor();
    }
    delete dlgColorSpaceConversion;
}

void ColorSpaceConversion::slotLayerColorSpaceConversion()
{

    KisImageSP image = m_view -> getCanvasSubject() -> currentImg();
    if (!image) return;

    KisPaintDeviceImplSP dev = image -> activeDevice();
    if (!dev) return;

    DlgColorSpaceConversion * dlgColorSpaceConversion = new DlgColorSpaceConversion(m_view, "ColorSpaceConversion");
    Q_CHECK_PTR(dlgColorSpaceConversion);

    dlgColorSpaceConversion -> setCaption(i18n("Convert Current Layer From") + dev -> colorSpace() -> id().name());
    dlgColorSpaceConversion -> fillCmbSrcProfile(dev -> colorSpace() -> id());

    KisProfile *  p = dev -> profile();
    if ( p ) {
        dlgColorSpaceConversion -> m_page -> cmbSourceProfile -> setCurrentText(p -> productName());
    }

    if (dlgColorSpaceConversion -> exec() == QDialog::Accepted) {
        KisID cspace = dlgColorSpaceConversion -> m_page -> cmbColorSpaces -> currentItem();
        KisColorSpace * cs = KisColorSpaceRegistry::instance() -> get(cspace);
        KisColorSpaceRegistry::instance()->getProfileByName(dlgColorSpaceConversion -> m_page -> cmbSourceProfile -> currentText());
        QApplication::setOverrideCursor(KisCursor::waitCursor());
        dev -> convertTo(cs,
                         KisColorSpaceRegistry::instance()->getProfileByName(dlgColorSpaceConversion -> m_page -> cmbDestProfile -> currentText()),
                         dlgColorSpaceConversion -> m_page -> grpIntent -> selectedId());
        QApplication::restoreOverrideCursor();
        image -> notify();
        image -> notifyLayersChanged();
    }
    delete dlgColorSpaceConversion;
}

#include "colorspaceconversion.moc"
