/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2016 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KISSWATCH_H
#define KISSWATCH_H

#include "kritapigment_export.h"
#include "KoColor.h"
#include "KoColorModelStandardIds.h"
#include <QString>

class KRITAPIGMENT_EXPORT KisSwatch
{
public:
    KisSwatch() = default;
    KisSwatch(const KoColor &color, const QString &name = QString());

public:
    QString name() const { return m_name; }
    void setName(const QString &name);

    QString id() const { return m_id; }
    void setId(const QString &id);

    KoColor color() const { return m_color; }
    void setColor(const KoColor &color);

    bool spotColor() const { return m_spotColor; }
    void setSpotColor(bool spotColor);

    bool isValid() const { return m_valid; }


    void writeToStream(QDataStream& stream, const QString& groupName, int originalRow , int originalColumn);
    static KisSwatch fromByteArray(QByteArray& data, QString &groupName, int &originalRow, int &originalColumn);
    static KisSwatch fromByteArray(QByteArray &data);

public:
    bool operator==(const KisSwatch& rhs) const {
        return m_color == rhs.m_color && m_name == rhs.m_name;
    }

private:
    KoColor m_color;
    QString m_name;
    QString m_id;
    bool m_spotColor {false};
    bool m_valid {false};
};

#endif // KISSWATCH_H
