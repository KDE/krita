/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2016 Michael Abrahams <miabraha@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
    ~KisSelectionModifierMapper() override;
    static KisSelectionModifierMapper *instance();
    static SelectionAction map(Qt::KeyboardModifiers m);

public Q_SLOTS:
    void slotConfigChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
