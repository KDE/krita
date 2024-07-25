/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushTypeMetaDataFixup.h"

#include "kis_debug.h"

#include <QSqlQuery>
#include <QSqlError>

#include <KisResourceLocator.h>
#include "kis_brush.h"


QStringList KisBrushTypeMetaDataFixup::executeFix()
{
    QStringList errorMessages;

    QSqlQuery q;
    const bool r = q.prepare("SELECT resources.id FROM resources "
              "INNER JOIN resource_types ON resources.resource_type_id = resource_types.id "
              "LEFT JOIN metadata ON metadata.foreign_id = resources.id AND metadata.key = :metadata_key "
              "WHERE resource_types.name = :resource_type AND metadata.value IS Null;");
    if (!r) {
        errorMessages.append(i18n("Could not access brush tip metadata"));
        return errorMessages;
    }
    q.bindValue(":resource_type", ResourceType::Brushes);
    q.bindValue(":metadata_key", KisBrush::brushTypeMetaDataKey);

    if (!q.exec()) {
        errorMessages.append(i18n("Could not access brush tip metadata"));
    }
    else {
        bool updatedAtLeastOneResource = false;

        while(q.next()) {
            /// we cannot use KisResourceModel here because it
            /// is not yet initialized at this state
            KoResourceSP res = KisResourceLocator::instance()->resourceForId(q.value(0).toInt());
            KIS_SAFE_ASSERT_RECOVER(res) { continue; }
            KisBrushSP brush = res.dynamicCast<KisBrush>();
            KIS_SAFE_ASSERT_RECOVER(brush) { continue; }

            /// on loading the metadata of the brush has been
            /// initialized properly, so we can just write the
            /// updated version into the database back...
            KisResourceLocator::instance()->setMetaDataForResource(res->resourceId(), res->metadata());

            updatedAtLeastOneResource = true;
        }

        if (updatedAtLeastOneResource) {
            qWarning() << "Successfully updated brush type metadata in the database";
        }
    }

    return errorMessages;
}
