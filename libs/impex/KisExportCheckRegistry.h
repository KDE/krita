/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISEXPORTCHECKREGISTRY_H
#define KISEXPORTCHECKREGISTRY_H

#include <QString>

#include <KoGenericRegistry.h>

#include "KisExportCheckBase.h"

#include "kritaimpex_export.h"


class KRITAIMPEX_EXPORT KisExportCheckRegistry : public QObject, public KoGenericRegistry<KisExportCheckFactory*>
{
public:
    KisExportCheckRegistry();
    ~KisExportCheckRegistry() override;
    static KisExportCheckRegistry *instance();

private:
    Q_DISABLE_COPY(KisExportCheckRegistry)
};


#endif // KISEXPORTCHECKREGISTRY_H
