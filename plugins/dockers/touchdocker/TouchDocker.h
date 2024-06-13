/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _TOUCH_DOCKER_H_
#define _TOUCH_DOCKER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class TouchDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        TouchDockerPlugin(QObject *parent, const QVariantList &);
        ~TouchDockerPlugin() override;
};

#endif /* _TOUCH_DOCKER_H_ */
