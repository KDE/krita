/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _PRESET_DOCKER_H_
#define _PRESET_DOCKER_H_

#include <QObject>
#include <QVariant>

/**
 * Shows the last used presets
 */
class PresetHistoryPlugin : public QObject
{
    Q_OBJECT
    public:
        PresetHistoryPlugin(QObject *parent, const QVariantList &);
        ~PresetHistoryPlugin() override;
};

#endif
