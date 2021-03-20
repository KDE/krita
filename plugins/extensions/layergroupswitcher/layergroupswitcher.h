/*
 * layergroupswitcher.h -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2013 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef LAYERGROUP_SWITCHER_H
#define LAYERGROUP_SWITCHER_H

#include <QVariant>

#include <KisActionPlugin.h>

class LayerGroupSwitcher : public KisActionPlugin
{
    Q_OBJECT
public:
    LayerGroupSwitcher(QObject *parent, const QVariantList &);
    ~LayerGroupSwitcher() override;

private Q_SLOTS:

    void moveIntoNextGroup();
    void moveIntoPreviousGroup();
};

#endif // LAYERGROUP_SWITCHER_H
