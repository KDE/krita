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

/**
 * This is a basic template to create selection tools from basic path based drawing tools.
 * The template overrides the ability to execute alternate actions correctly.
 * Modifier keys are overridden with the following behavior:
 *
 * Shift: add to selection
 * Alt: subtract from selection
 * Shift+Alt: intersect current selection
 * Ctrl: replace selection
 *
 * Certain tools also use modifier keys to alter their behavior, e.g. forcing square proportions with the rectangle tool.
 * The template enables the following rules for forwarding keys:
 * 1) Any modifier keys held *when the tool is first activated* will determine the new selection method.
 * 2) If the underlying tool *does not take modifier keys*, pressing modifier keys in the middle of a stroke will change the selection method.  This applies to the lasso tool and polygon tool.
 * 3) If the underlying tool *takes modifier keys,* they will always be forwarded to the underlying tool, and it is not possible to change the selection method in the middle of a stroke.
 */

#include "kis_selection.h"
#include "kis_selection_modifier_mapper.h"
#include "kis_config_notifier.h"
#include "kis_config.h"

Q_GLOBAL_STATIC(KisSelectionModifierMapper, s_instance)


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
    Qt::KeyboardModifiers symmetricdifferenceModifiers;
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
    KisConfig cfg(true);
    if (!cfg.switchSelectionCtrlAlt()) {
        replaceModifiers   = Qt::ControlModifier;
        intersectModifiers = (Qt::KeyboardModifiers)(Qt::AltModifier | Qt::ShiftModifier);
        subtractModifiers  = Qt::AltModifier;
        symmetricdifferenceModifiers = (Qt::KeyboardModifiers)(Qt::ControlModifier | Qt::AltModifier);
    } else {
        replaceModifiers   = Qt::AltModifier;
        intersectModifiers = (Qt::KeyboardModifiers)(Qt::ControlModifier | Qt::ShiftModifier);
        subtractModifiers  = Qt::ControlModifier;
        symmetricdifferenceModifiers = (Qt::KeyboardModifiers)(Qt::ShiftModifier | Qt::ControlModifier);
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
    } else if (m == symmetricdifferenceModifiers) {
        newAction = SELECTION_SYMMETRICDIFFERENCE;
    }
        
    return newAction;
}
