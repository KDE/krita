/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PARTICLE_PAINTOP_PLUGIN_H_
#define PARTICLE_PAINTOP_PLUGIN_H_

#include <QObject>
#include <QVariantList>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class ParticlePaintOpPlugin : public QObject
{
    Q_OBJECT
public:
    ParticlePaintOpPlugin(QObject *parent, const QVariantList &);
    ~ParticlePaintOpPlugin() override;
};

#endif // PARTICLE_PAINTOP_PLUGIN_H_
