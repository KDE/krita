/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_phf_import.h"

#include <QFileInfo>

#include <kpluginfactory.h>
#include <KoDialog.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>

#include "kis_debug.h"
#include "KisDocument.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_iterator_ng.h"

#include "../tiff/kis_tiff_converter.h"



K_PLUGIN_FACTORY_WITH_JSON(KisRawImportFactory, "krita_phf_import.json",
                           registerPlugin<KisRawImport>();)

KisRawImport::KisRawImport(QObject *parent, const QVariantList &)
        : KisImportExportFilter(parent)
{
    m_dialog = new KoDialog();
    m_dialog->enableButtonApply(false);
    QWidget* widget = new QWidget;
    m_rawWidget.setupUi(widget);
    m_dialog->setMainWidget(widget);
    //connect(m_rawWidget.pushButtonUpdate, SIGNAL(clicked()), this, SLOT(slotUpdatePreview()));
}

KisRawImport::~KisRawImport()
{
    delete m_dialog;
}

inline quint16 correctIndian(quint16 v)
{
#if KDCRAW_VERSION < 0x000400
    return ((v & 0x00FF) << 8) | ((v & 0xFF00 >> 8));
#else
    return v;
#endif
}

KisImportExportFilter::ConversionStatus KisRawImport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP /*configuration*/)
{
    if (filename().isEmpty()) {
        return KisImportExportFilter::FileNotFound;
    }

    QByteArray ba = filename().toLatin1();
     const char *c_str = ba.data(); 
    
    std::cout<<"filename: "<<c_str<<std::endl;
    
    char cmd[1001];
    snprintf(cmd, 1000, "photoflow --plugin \"%s\" /tmp/phf.tif /tmp/phf.pfi", c_str);
    printf("cmd: %s\n", cmd);
    system(cmd);

    
    QString ofilename = "/tmp/phf.tif";

    if (!QFileInfo(ofilename).exists()) {
        //return KisImportExportFilter::FileNotFound;
        return KisImportExportFilter::UserCancelled;
    }

    KisTIFFConverter ib(document);

//        if (view != 0)
//            view -> canvasSubject() ->  progressDisplay() -> setSubject(&ib, false, true);

    switch (ib.buildImage(ofilename)) {
    case KisImageBuilder_RESULT_UNSUPPORTED:
    	unlink("/tmp/phf.tif");
        return KisImportExportFilter::NotImplemented;
    case KisImageBuilder_RESULT_INVALID_ARG:
    	unlink("/tmp/phf.tif");
        return KisImportExportFilter::BadMimeType;
    case KisImageBuilder_RESULT_NO_URI:
    case KisImageBuilder_RESULT_NOT_LOCAL:
    	unlink("/tmp/phf.tif");
        return KisImportExportFilter::FileNotFound;
    case KisImageBuilder_RESULT_BAD_FETCH:
    case KisImageBuilder_RESULT_EMPTY:
    	unlink("/tmp/phf.tif");
        return KisImportExportFilter::ParsingError;
    case KisImageBuilder_RESULT_FAILURE:
        return KisImportExportFilter::InternalError;
    case KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE:
    	unlink("/tmp/phf.tif");
        return KisImportExportFilter::WrongFormat;
    case KisImageBuilder_RESULT_OK:
    	printf("calling doc -> setCurrentImage(ib.image())\n");
        document -> setCurrentImage(ib.image());
    	printf("doc -> setCurrentImage(ib.image()) finished\n");
    	unlink("/tmp/phf.tif");
        return KisImportExportFilter::OK;
    default:
    	unlink("/tmp/phf.tif");
        break;
    }

	unlink("/tmp/phf.tif");
    QApplication::restoreOverrideCursor();
    return KisImportExportFilter::UserCancelled;
}

void KisRawImport::slotUpdatePreview()
{
    QByteArray imageData;
}



#include "kis_phf_import.moc"
