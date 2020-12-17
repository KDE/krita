/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _LUT_DOCKER_H_
#define _LUT_DOCKER_H_

#include <QObject>
#include <QVariant>


/**
 * Template of view plugin
 */
class LutDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        LutDockerPlugin(QObject *parent, const QVariantList &);
        virtual ~LutDockerPlugin();
};

#endif
