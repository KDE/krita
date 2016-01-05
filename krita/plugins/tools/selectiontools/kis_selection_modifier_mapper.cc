/* This file is part of the KDE project
 * Copyright (C) 2016 Michael Abrahams <miabraha@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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

#include "kis_selection.h"
#include "kis_selection_modifier_mapper.h"
#include "kis_config_notifier.h"
#include "kis_config.h"

Q_GLOBAL_STATIC(KisSelectionModifierMapper, s_instance);


// This numerically serializes modifier flags... let's keep it around for later.
#if 0
#include <bitset>
QString QMOD_BINARY(Qt::KeyboardModifiers m)
{
    return QString(std::bitset<sizeof(int) * 8>(m).to_string().c_str());
};
#endif

struct Q_DECL_HIDDEN KisSelectionModifierMapper::Private
{
    SelectionAction map(Qt::KeyboardModifiers m);
    void slotConfigChanged();
    Qt::KeyboardModifiers replaceModifiers;
    Qt::KeyboardModifiers intersectModifiers;
    Qt::KeyboardModifiers addModifiers;
    Qt::KeyboardModifiers subtractModifiers;
};


KisSelectionModifierMapper::KisSelectionModifierMapper()
    : m_d(new Private)
{
    connect(KisConfigNotifier::instance(),
            SIGNAL(configChanged()),
            SLOT(slotConfigChanged()));
    slotConfigChanged();
}


KisSelectionModifierMapper::~KisSelectionModifierMapper()
{
}

KisSelectionModifierMapper *KisSelectionModifierMapper::instance()
{
    return s_instance;
}

void KisSelectionModifierMapper::slotConfigChanged()
{
    m_d->slotConfigChanged();
}


void KisSelectionModifierMapper::Private::slotConfigChanged()
{
    KisConfig cfg;
    if (!cfg.switchSelectionCtrlAlt()) {
        replaceModifiers   = Qt::ControlModifier;
        intersectModifiers = (Qt::KeyboardModifiers)(Qt::AltModifier | Qt::ShiftModifier);
        subtractModifiers  = Qt::AltModifier;
    } else {
        replaceModifiers   = Qt::AltModifier;
        intersectModifiers = (Qt::KeyboardModifiers)(Qt::ControlModifier | Qt::ShiftModifier);
        subtractModifiers  = Qt::ControlModifier;
    }

    addModifiers = Qt::ShiftModifier;
}

SelectionAction KisSelectionModifierMapper::map(Qt::KeyboardModifiers m)
{
    return s_instance->m_d->map(m);
}

SelectionAction KisSelectionModifierMapper::Private::map(Qt::KeyboardModifiers m)
{
    SelectionAction newAction = SELECTION_DEFAULT;
    if (m == replaceModifiers) {
        newAction = SELECTION_REPLACE;
    } else if (m == intersectModifiers) {
        newAction = SELECTION_INTERSECT;
    } else if (m == addModifiers) {
        newAction = SELECTION_ADD;
    } else if (m == subtractModifiers) {
        newAction = SELECTION_SUBTRACT;
    }
    return newAction;
}
