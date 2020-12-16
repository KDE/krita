/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _PRESET_DOCKER_H_
#define _PRESET_DOCKER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class PresetDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        PresetDockerPlugin(QObject *parent, const QVariantList &);
        ~PresetDockerPlugin() override;
    private:
        KisViewManager* m_view;
};

#endif
