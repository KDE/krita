/* This file is part of the Calligra project
 * SPDX-FileCopyrightText: 2008 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_COLOR_SOURCE_OPTION_H
#define _KIS_COLOR_SOURCE_OPTION_H

#include <kritapaintop_export.h>

#include <QList>

#include <kis_properties_configuration.h>

class KisColorSource;
class KoID;
class KisPainter;

class PAINTOP_EXPORT KisColorSourceOption
{
public:
    enum Type {
        PLAIN,
        GRADIENT,
        UNIFORM_RANDOM,
        TOTAL_RANDOM,
        PATTERN,
        PATTERN_LOCKED
    };
public:
    KisColorSourceOption();
    ~KisColorSourceOption();
    void writeOptionSetting(KisPropertiesConfigurationSP setting) const;
    void readOptionSetting(const KisPropertiesConfigurationSP setting);

    KisColorSource* createColorSource(const KisPainter* _painter) const;
    QString colorSourceTypeId() const;
    void setColorSourceType(Type _type);
    void setColorSourceType(const QString& _type);
    static QList<KoID> sourceIds();

    Type type() const;

private:
    struct Private;
    Private* const d;
};

#endif
