/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisPreExportChecker.h"
#include "KisExportCheckBase.h"
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

        KisExportCheckFactory *factory = KisExportCheckRegistry::instance()->get(id);
        KisExportCheckBase *check = factory->create(KisExportCheckBase::SUPPORTED);

        if (!doPerLayerChecks && check->perLayerCheck()) {
            continue;
        }

        if (check->checkNeeded(image)) {
            if (!filterChecks.contains(id)) {
                m_warnings << check->warning();
            }
            else {
                KisExportCheckBase *filterCheck = filterChecks[id];
                KisExportCheckBase::Level level = filterCheck->check(image);
                QString warning = filterCheck->warning();

                if (level == KisExportCheckBase::PARTIALLY) {
                    m_warnings << warning;
                }
                else if (level == KisExportCheckBase::UNSUPPORTED) {
                    m_errors << warning;
                }
                else {
                    continue;
                }
            }
        }
        delete check;
    }
    return m_warnings.isEmpty() && m_errors.isEmpty();
}

QStringList KisPreExportChecker::errors() const
{
    return m_errors;
}


QStringList KisPreExportChecker::warnings() const
{
    return m_warnings;
}

