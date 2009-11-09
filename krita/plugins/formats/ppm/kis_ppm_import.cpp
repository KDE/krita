/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_ppm_import.h"

#include <kgenericfactory.h>

#include <KoFilterChain.h>

#include <kis_debug.h>

typedef KGenericFactory<KisPPMImport> PPMImportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritappmimport, PPMImportFactory("kofficefilters"))

KisPPMImport::KisPPMImport(QObject* parent, const QStringList&) : KoFilter(parent)
{
}

KisPPMImport::~KisPPMImport()
{
}

KoFilter::ConversionStatus KisPPMImport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "Importing using JPEGImport!";

    if (to != "application/x-krita")
        return KoFilter::BadMimeType;
    
    abort();
}
