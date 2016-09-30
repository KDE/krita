/* This file is part of the KDE project
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisPreExportChecker.h"
#include "KisExportCheckBase.h"
#include "KisExportConverterBase.h"
#include "KisExportCheckRegistry.h"

#include <kis_image.h>

KisPreExportChecker::KisPreExportChecker()
{
    KisExportCheckRegistry::instance();
}

bool KisPreExportChecker::check(KisImageSP image, QMap<QString, KisExportCheckBase*> filterChecks)
{
    bool doPerLayerChecks = false;
    if (filterChecks.contains("MultiLayerCheck") && filterChecks["MultiLayerCheck"]->check(image) == KisExportCheckBase::SUPPORTED) {
        doPerLayerChecks = true;
    }

    Q_FOREACH(const QString &id, KisExportCheckRegistry::instance()->keys()) {
        KisExportCheckBase *check = createCheck(id, KisExportCheckBase::SUPPORTED);
        if (!doPerLayerChecks && check->perLayerCheck()) {
            continue;
        }
        if (check->checkNeeded(image)) {
            if (!filterChecks.contains(id)) {
                m_warnings << check->warning();
            }
            else if (filterChecks[id]->check(image) != KisExportCheckBase::SUPPORTED) {
                m_warnings << filterChecks[id]->warning();
            }
            else {
                continue;
            }
            if (check->converter()) {
                m_conversions[check->converter()->id()] = check->converter();
            }
        }
        delete check;
    }

    return m_warnings.isEmpty();
}

KisImageSP KisPreExportChecker::convertedImage(KisImageSP originalImage) const
{
    // Sort the conversions on priority
    // Copy the image
    // KisImageSP copy = new KisImage(originalImage.data());
    // Apply the conversions
    Q_FOREACH(KisExportConverterBase *converter, m_conversions.values()) {
        if (!converter->convert(originalImage)) {
            qDebug() << "Failed to apply conversion" << converter->id();
            return 0;
        }
    }
    return originalImage;
}

QStringList KisPreExportChecker::warnings() const
{
    return m_warnings;
}

bool KisPreExportChecker::conversionNeeded() const
{
    return !m_conversions.isEmpty();
}

KisExportCheckBase *KisPreExportChecker::createCheck(const QString &id, const KisExportCheckBase::Level level, const QString &customWarning)
{
    KisExportCheckFactory *factory = KisExportCheckRegistry::instance()->get(id);
    if (factory) {
        return factory->create(level, customWarning);
    }
    return 0;
}
