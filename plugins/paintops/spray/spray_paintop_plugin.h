/*
 *  SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SPRAY_PAINTOP_PLUGIN_H_
#define SPRAY_PAINTOP_PLUGIN_H_

#include <QObject>
#include <QVariant>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class SprayPaintOpPlugin : public QObject
{
    Q_OBJECT
public:
    SprayPaintOpPlugin(QObject *parent, const QVariantList &);
    ~SprayPaintOpPlugin() override;
};

#endif // SPRAY_PAINTOP_PLUGIN_H_
