/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef MultiLayerCheck_H
#define MultiLayerCheck_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <kis_group_layer.h>

class MultiLayerCheck : public KisExportCheckBase
{
public:

    MultiLayerCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "The image has <b>more than one layer or a mask or an active selection</b>. Only the flattened image will be saved.");
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        return (image->rootLayer()->childCount() > 1);
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }

};

class MultiLayerCheckFactory : public KisExportCheckFactory
{
public:

    MultiLayerCheckFactory() {}

    ~MultiLayerCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new MultiLayerCheck(id(), level, customWarning);
    }

    QString id() const override {
        return "MultiLayerCheck";
    }
};

#endif // MultiLayerCheck_H
