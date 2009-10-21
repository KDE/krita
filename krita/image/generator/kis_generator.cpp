/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "generator/kis_generator.h"

#include <QString>

#include <KoProgressUpdater.h>

#include "kis_bookmarked_configuration_manager.h"
#include "filter/kis_filter_configuration.h"
#include "kis_processing_information.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_types.h"


KisGenerator::KisGenerator(const KoID& _id, const KoID & category, const QString & entry)
        : KisBaseProcessor(_id, category, entry)
{
    init(id() + "_generator_bookmarks");
}

KisGenerator::~KisGenerator()
{
}

void KisGenerator::generate(KisProcessingInformation dst,
                            const QSize& size,
                            const KisFilterConfiguration* config
                           ) const
{
    generate(dst, size, config, 0);
}

const KoColorSpace * KisGenerator::colorSpace()
{
    return 0;
}

QRect KisGenerator::generatedRect(QRect _imageArea, const KisFilterConfiguration*) const
{
    return _imageArea;
}
