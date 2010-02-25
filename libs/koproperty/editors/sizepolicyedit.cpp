/* This file is part of the KDE project
   Copyright (C) 2008-2009 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "sizepolicyedit.h"

#include <QtGui/QSizePolicy>
#include <KLocale>
#include <KGlobal>

using namespace KoProperty;

class SizePolicyListData : public Property::ListData
{
public:
    SizePolicyListData() : Property::ListData(keysInternal(), stringsInternal())
    {
    }

    QString nameForPolicy(QSizePolicy::Policy p) {
        const int index = keys.indexOf((int)p);
        if (index == -1)
            return names[0];
        return names[index];
    }
private:
    static QList<QVariant> keysInternal() {
        QList<QVariant> keys;
        keys
         << QSizePolicy::Fixed
         << QSizePolicy::Minimum
         << QSizePolicy::Maximum
         << QSizePolicy::Preferred
         << QSizePolicy::Expanding
         << QSizePolicy::MinimumExpanding
         << QSizePolicy::Ignored;
        return keys;
    }

    static QStringList stringsInternal() {
        QStringList strings;
        strings
         << i18nc("Size Policy", "Fixed")
         << i18nc("Size Policy", "Minimum")
         << i18nc("Size Policy", "Maximum")
         << i18nc("Size Policy", "Preferred")
         << i18nc("Size Policy", "Expanding")
         << i18nc("Size Policy", "Minimum Expanding")
         << i18nc("Size Policy", "Ignored");
        return strings;
    }
};

K_GLOBAL_STATIC(SizePolicyListData, s_sizePolicyListData)

//---------

static const char *SIZEPOLICY_MASK = "%1, %2, %3, %4";

QString SizePolicyDelegate::displayText( const QVariant& value ) const
{
    const QSizePolicy sp(value.value<QSizePolicy>());
    
    return QString::fromLatin1(SIZEPOLICY_MASK)
        .arg(s_sizePolicyListData->nameForPolicy(sp.horizontalPolicy()))
        .arg(s_sizePolicyListData->nameForPolicy(sp.verticalPolicy()))
        .arg(sp.horizontalStretch())
        .arg(sp.verticalStretch());
}

//static
const Property::ListData& SizePolicyDelegate::listData()
{
    return *s_sizePolicyListData;
}

//------------

SizePolicyComposedProperty::SizePolicyComposedProperty(Property *property)
        : ComposedPropertyInterface(property)
{
    (void)new Property("hor_policy", new SizePolicyListData(),
        QVariant(), i18n("Hor. Policy"), i18n("Horizontal Policy"), ValueFromList, property);
    (void)new Property("vert_policy", new SizePolicyListData(),
        QVariant(), i18n("Vert. Policy"), i18n("Vertical Policy"), ValueFromList, property);
    (void)new Property("hor_stretch", QVariant(),
        i18n("Hor. Stretch"), i18n("Horizontal Stretch"), UInt, property);
    (void)new Property("vert_stretch", QVariant(),
        i18n("Vert. Stretch"), i18n("Vertical Stretch"), UInt, property);
}

void SizePolicyComposedProperty::setValue(Property *property, 
    const QVariant &value, bool rememberOldValue)
{
    const QSizePolicy sp( value.value<QSizePolicy>() );
    property->child("hor_policy")->setValue(sp.horizontalPolicy(), rememberOldValue, false);
    property->child("vert_policy")->setValue(sp.verticalPolicy(), rememberOldValue, false);
    property->child("hor_stretch")->setValue(sp.horizontalStretch(), rememberOldValue, false);
    property->child("vert_stretch")->setValue(sp.verticalStretch(), rememberOldValue, false);
}

void SizePolicyComposedProperty::childValueChanged(Property *child,
    const QVariant &value, bool rememberOldValue)
{
    QSizePolicy sp( child->parent()->value().value<QSizePolicy>() );
    if (child->name() == "hor_policy")
        sp.setHorizontalPolicy(static_cast<QSizePolicy::Policy>(value.toInt()));
    else if (child->name() == "vert_policy")
        sp.setVerticalPolicy(static_cast<QSizePolicy::Policy>(value.toInt()));
    else if (child->name() == "hor_stretch")
        sp.setHorizontalStretch(value.toInt());
    else if (child->name() == "vert_stretch")
        sp.setVerticalStretch(value.toInt());

    child->parent()->setValue(sp, true, false);
}
