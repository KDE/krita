/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "generator/kis_generator.h"

#include <QString>

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
                            const KisFilterConfigurationSP config
                           ) const
{
    generate(dst, size, config, 0);
}

QRect KisGenerator::generatedRect(QRect _imageArea, const KisFilterConfigurationSP) const
{
    return _imageArea;
}
