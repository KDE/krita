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

#include "KisExportCheckRegistry.h"


class ColorModelCheck : public KisExportCheckBase
{
public:

    ColorModelCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
    {}

    bool checkNeeded(KisImageSP image) const
    {
        return true;
    }

    Level check(KisImageSP image) const
    {
        return KisExportCheckBase::SUPPORTED;
    }
};

class ColorModelCheckFactory : public KisExportCheckFactory
{
    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning)
    {
        return new ColorModelCheck(id(), level, customWarning);
    }

    QString id() const {
        return "ColorModelCheck";
    }
};


#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisExportCheckRegistry, s_instance)

KisExportCheckRegistry::KisExportCheckRegistry ()
{
    KisExportCheckFactory *check = 0;

    check = new ColorModelCheckFactory;
    add(check->id(), check);
}

KisExportCheckRegistry::~KisExportCheckRegistry ()
{
    qDeleteAll(values());
}

KisExportCheckRegistry *KisExportCheckRegistry ::instance()
{
    return s_instance;
}

