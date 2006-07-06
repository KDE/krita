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

#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QApplication>
#include <QCursor>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include "kis_doc.h"
#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_meta_registry.h"
#include "kis_view.h"
#include "kis_paint_device.h"
#include "KoColorSpaceFactoryRegistry.h"
#include "kis_cmb_idlist.h"
#include "squeezedcombobox.h"

#include "colorspaceconversion.h"
#include "dlg_colorspaceconversion.h"

typedef KGenericFactory<ColorSpaceConversion> ColorSpaceConversionFactory;
K_EXPORT_COMPONENT_FACTORY( kritacolorspaceconversion, ColorSpaceConversionFactory( "krita" ) )


ColorSpaceConversion::ColorSpaceConversion(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView") )
    {
        m_view = (KisView*) parent;

        setInstance(ColorSpaceConversionFactory::instance());
        
setXMLFile(KStandardDirs::locate("data","kritaplugins/colorspaceconversion.rc"), 
true);

        KAction *action = new KAction(i18n("&Convert Image Type..."), actionCollection(), "imgcolorspaceconversion");
        connect(action, SIGNAL(triggered()), this, SLOT(slotImgColorSpaceConversion()));
        action = new KAction(i18n("&Convert Layer Type..."), actionCollection(), "layercolorspaceconversion");
        connect(action, SIGNAL(triggered()), this, SLOT(slotLayerColorSpaceConversion()));
    }
}

ColorSpaceConversion::~ColorSpaceConversion()
{
    m_view = 0;
}

void ColorSpaceConversion::slotImgColorSpaceConversion()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;


    if (image->colorSpace()->willDegrade(TO_LAB16)) {
        if (KMessageBox::warningContinueCancel(m_view,
            i18n("This conversion will convert your %1 image through 16-bit L*a*b* and back.\n"
                    "Watercolor and openEXR colorspaces will even be converted through 8-bit RGB.\n"
                    , image->colorSpace()->id().name()),
            i18n("Colorspace Conversion"),
            KGuiItem(i18n("Continue")),
            "lab16degradation") != KMessageBox::Continue) return;

    }

    DlgColorSpaceConversion * dlgColorSpaceConversion = new DlgColorSpaceConversion(m_view, "ColorSpaceConversion");
    Q_CHECK_PTR(dlgColorSpaceConversion);

    dlgColorSpaceConversion->setCaption(i18n("Convert All Layers From ") + image->colorSpace()->id().name());

    if (dlgColorSpaceConversion->exec() == QDialog::Accepted) {
        // XXX: Do the rest of the stuff
        KoID cspace = dlgColorSpaceConversion->m_page->cmbColorSpaces->currentItem();
        KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getColorSpace(cspace, dlgColorSpaceConversion->m_page->cmbDestProfile->currentText());

        QApplication::setOverrideCursor(KisCursor::waitCursor());
        image->convertTo(cs, dlgColorSpaceConversion->m_intentButtonGroup.checkedId());
        QApplication::restoreOverrideCursor();
    }
    delete dlgColorSpaceConversion;
}

void ColorSpaceConversion::slotLayerColorSpaceConversion()
{

    KisImageSP image = m_view->canvasSubject()->currentImg();
    if (!image) return;

    KisPaintDeviceSP dev = image->activeDevice();
    if (!dev) return;

    if (dev->colorSpace()->willDegrade(TO_LAB16)) {
        if (KMessageBox::warningContinueCancel(m_view,
            i18n("This conversion will convert your %1 layer through 16-bit L*a*b* and back.\n"
                    "Watercolor and openEXR colorspaces will even be converted through 8-bit RGB.\n"
                    , dev->colorSpace()->id().name()),
            i18n("Colorspace Conversion"),
            KGuiItem(i18n("Continue")),
            "lab16degradation") != KMessageBox::Continue) return;

    }

    DlgColorSpaceConversion * dlgColorSpaceConversion = new DlgColorSpaceConversion(m_view, "ColorSpaceConversion");
    Q_CHECK_PTR(dlgColorSpaceConversion);

    dlgColorSpaceConversion->setCaption(i18n("Convert Current Layer From") + dev->colorSpace()->id().name());

    if (dlgColorSpaceConversion->exec() == QDialog::Accepted) {
        KoID cspace = dlgColorSpaceConversion->m_page->cmbColorSpaces->currentItem();
        KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry() ->
                getColorSpace(cspace, dlgColorSpaceConversion->m_page->cmbDestProfile->currentText());

        QApplication::setOverrideCursor(KisCursor::waitCursor());
        dev->convertTo(cs, dlgColorSpaceConversion->m_intentButtonGroup.checkedId());
        QApplication::restoreOverrideCursor();
    }
    delete dlgColorSpaceConversion;
}

#include "colorspaceconversion.moc"
