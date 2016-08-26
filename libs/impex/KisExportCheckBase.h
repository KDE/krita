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
#ifndef KISEXPORTCHECKBASE_H
#define KISEXPORTCHECKBASE_H

#include <kis_types.h>

#include <QString>

class KisExportConverterBase;

/**
 * @brief The KisExportCheckBase class defines the interface
 * of the individual checks of an export filter's capabilities
 */
class KisExportCheckBase
{
public:

    /// The level determines the level of support the export
    /// filter has for the given feature.
    enum Level {
        SUPPORTED,  //< The filter fully supports this
        PARTIALLY,  //< The filter can handle this, but the user needs to be warned about possible degradation
        UNSUPPORTED //< This cannot be saved using this filter
    };

    /**
     * @brief KisExportCheckBase
     * @param level the level of support the filter has for the given feature
     * @param customWarning A custom warning to use instead of the default one
     */
    KisExportCheckBase(Level level, const QString &customWarning = QString())
        : m_converter(0)
    {
        if (!customWarning.isEmpty()) {
            m_message = customWarning;
        }
    }

    virtual ~KisExportCheckBase()
    {
        delete m_converter;
    }

    /// @return the unique id of the check
    virtual QString id() const = 0;

    /// @return whether the image uses this feature
    virtual bool checkNeeded(KisImageSP image) const = 0;

    /// @return the level of support for this feature
    virtual Level check() const = 0;

    /// @return the message to show the user
    QString message() const { return m_message; }

    KisExportConverterBase *converter() const { return m_converter; }

private:

    QString m_message;
    KisExportConverterBase *m_converter;

};

#endif

QString KisExportCheckBase::message() const
{
    return m_message;
}
