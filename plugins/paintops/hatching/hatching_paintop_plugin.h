/*
 *  SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef HATCHING_PAINTOP_PLUGIN_H_
#define HATCHING_PAINTOP_PLUGIN_H_

#include <QObject>
#include <QVariant>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class HatchingPaintOpPlugin : public QObject
{
    Q_OBJECT
public:
    HatchingPaintOpPlugin(QObject *parent, const QVariantList &);
    ~HatchingPaintOpPlugin() override;
};

#endif // HATCHING_PAINTOP_PLUGIN_H_
