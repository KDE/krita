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

#ifndef CompositionsCHECK_H
#define CompositionsCHECK_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>

class CompositionsCheck : public KisExportCheckBase
{
public:

    CompositionsCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "The image contains <b>compositions</b>. The compositions will not be saved.");
        }
    }

    bool checkNeeded(KisImageSP image) const
    {
        return (image->compositions().size() > 0);
    }

    Level check(KisImageSP /*image*/) const
    {
        return m_level;
    }
};

class CompositionsCheckFactory : public KisExportCheckFactory
{
public:

    CompositionsCheckFactory()
    {
    }

    virtual ~CompositionsCheckFactory() {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning)
    {
        return new CompositionsCheck(id(), level, customWarning);
    }

    QString id() const {
        return "CompositionsCheck";
    }
};

#endif // CompositionsCHECK_H
