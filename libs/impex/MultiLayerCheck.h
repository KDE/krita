/*
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
