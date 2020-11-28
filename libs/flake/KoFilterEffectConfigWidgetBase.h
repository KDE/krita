/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KOFILTEREFFECTCONFIGWIDGETBASE_H
#define KOFILTEREFFECTCONFIGWIDGETBASE_H

#include "kritaflake_export.h"
#include <QWidget>

class KoFilterEffect;

/// Base class for filter effects config widgets
class KRITAFLAKE_EXPORT KoFilterEffectConfigWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit KoFilterEffectConfigWidgetBase(QWidget *parent = 0);
    ~KoFilterEffectConfigWidgetBase() override {};

    /// Sets the filter effect to be edited by the config widget
    virtual bool editFilterEffect(KoFilterEffect *filterEffect) = 0;

Q_SIGNALS:
    /// Is emitted when the filter effect has changed
    void filterChanged();
};

#endif // KOFILTEREFFECTCONFIGWIDGETBASE_H
