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

#ifndef KIS_SELECTION_MODIFIER_MAPPER_H_
#define KIS_SELECTION_MODIFIER_MAPPER_H_

/**
 * See KisToolSelectBase for usage.
 */

#include "kis_selection.h"
#include <QScopedPointer>

class KisSelectionModifierMapper : public QObject
{
    Q_OBJECT

public:
    KisSelectionModifierMapper();
    ~KisSelectionModifierMapper();
    static KisSelectionModifierMapper *instance();
    static SelectionAction map(Qt::KeyboardModifiers m);

public Q_SLOTS:
    void slotConfigChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
