/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _BRUSHHUD_DOCKER_H_
#define _BRUSHHUD_DOCKER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class BrushHudDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        BrushHudDockerPlugin(QObject *parent, const QVariantList &);
        ~BrushHudDockerPlugin() override;
};

#endif
