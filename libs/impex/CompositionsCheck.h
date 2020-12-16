/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

    bool checkNeeded(KisImageSP image) const override
    {
        return (image->compositions().size() > 0);
    }

    Level check(KisImageSP /*image*/) const override
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

    ~CompositionsCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new CompositionsCheck(id(), level, customWarning);
    }

    QString id() const override {
        return "CompositionsCheck";
    }
};

#endif // CompositionsCHECK_H
