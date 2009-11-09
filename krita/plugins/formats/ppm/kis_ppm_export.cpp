/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#include "kis_ppm_export.h"

#include <KGenericFactory>
#include <KoFilterChain.h>

#include <kis_debug.h>

typedef KGenericFactory<KisPPMExport> KisPPMExportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritappmexport, KisPPMExportFactory("kofficefilters"))

KisPPMExport::KisPPMExport(QObject *parent, const QStringList&) : KoFilter(parent)
{
}

KisPPMExport::~KisPPMExport()
{
}

KoFilter::ConversionStatus KisPPMExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "PPM export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;
    abort();
}
