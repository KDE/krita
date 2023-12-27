/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisColorSourceOptionData.h"

#include <klocalizedstring.h>
#include <KoID.h>
#include <kis_properties_configuration.h>

namespace {

struct ColorSourceTypeMapper {
    ColorSourceTypeMapper() {
        addType(KisColorSourceOptionData::PLAIN, KoID("plain", i18n("Plain color")));
        addType(KisColorSourceOptionData::GRADIENT, KoID("gradient", i18n("Gradient")));
        addType(KisColorSourceOptionData::UNIFORM_RANDOM, KoID("uniform_random", i18n("Uniform random")));
        addType(KisColorSourceOptionData::TOTAL_RANDOM, KoID("total_random", i18n("Total random")));
        addType(KisColorSourceOptionData::PATTERN, KoID("pattern", i18n("Pattern")));
        addType(KisColorSourceOptionData::PATTERN_LOCKED, KoID("lockedpattern", i18n("Locked pattern")));
    }
    QMap<KisColorSourceOptionData::Type, KoID> type2id;
    QMap<QString, KisColorSourceOptionData::Type> id2type;

private:
    void addType(KisColorSourceOptionData::Type _type, KoID _id) {
        type2id[_type] = _id;
        id2type[_id.id()] = _type;
    }
};

}

Q_GLOBAL_STATIC(ColorSourceTypeMapper, s_instance)

bool KisColorSourceOptionData::read(const KisPropertiesConfiguration *setting)
{
    const QString colorSourceType = setting->getString("ColorSource/Type", "plain");
    type = s_instance->id2type.value(colorSourceType, PLAIN);
    return true;
}

void KisColorSourceOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty("ColorSource/Type", s_instance->type2id.value(type).id());
}

QVector<KoID> KisColorSourceOptionData::colorSourceTypeIds()
{
    return s_instance->type2id.values().toVector();
}

KoID KisColorSourceOptionData::type2Id(Type type)
{
    return s_instance->type2id[type];
}

KisColorSourceOptionData::Type KisColorSourceOptionData::id2Type(const KoID &id)
{
    return s_instance->id2type[id.id()];
}
