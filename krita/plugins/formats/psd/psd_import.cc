/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#include "psd_import.h"

#include <kgenericfactory.h>

#include <KoFilterChain.h>

#include <kis_doc2.h>
#include <kis_image.h>

#include "psd_loader.h"

typedef KGenericFactory<psdImport> ImportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritapsdimport, ImportFactory("kofficefilters"))

        psdImport::psdImport(QObject *parent, const QStringList&) : KoFilter(parent)
{
}

psdImport::~psdImport()
{
}

KoFilter::ConversionStatus psdImport::convert(const QByteArray&, const QByteArray& to)
{
    dbgFile <<"Importing using PSDImport!";

    if (to != "application/x-krita")
        return KoFilter::BadMimeType;

    KisDoc2 * doc = dynamic_cast<KisDoc2*>(m_chain->outputDocument());

    if (!doc)
        return KoFilter::CreationError;

    QString filename = m_chain->inputFile();

    doc->prepareForImport();

    qDebug() << "filename" << filename;

    if (!filename.isEmpty()) {

        KUrl url;
        url.setPath(filename);

        if (url.isEmpty())
            return KoFilter::FileNotFound;

        PSDLoader ib(doc, doc->undoAdapter());

        KisImageBuilder_Result result = ib.buildImage(url);

        switch (result) {
        case KisImageBuilder_RESULT_UNSUPPORTED:
            return KoFilter::NotImplemented;
        case KisImageBuilder_RESULT_INVALID_ARG:
            return KoFilter::BadMimeType;
        case KisImageBuilder_RESULT_NO_URI:
        case KisImageBuilder_RESULT_NOT_EXIST:
        case KisImageBuilder_RESULT_NOT_LOCAL:
            return KoFilter::FileNotFound;
        case KisImageBuilder_RESULT_BAD_FETCH:
        case KisImageBuilder_RESULT_EMPTY:
            return KoFilter::ParsingError;
        case KisImageBuilder_RESULT_FAILURE:
            return KoFilter::InternalError;
        case KisImageBuilder_RESULT_OK:
            doc -> setCurrentImage( ib.image());
            return KoFilter::OK;
        default:
            qDebug() << "Result was: " << result;
        }

    }
    return KoFilter::StorageCreationError;
}

#include <psd_import.moc>

