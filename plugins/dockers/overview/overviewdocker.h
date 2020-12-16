/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _OVERVIEW_DOCKER_H_
#define _OVERVIEW_DOCKER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class OverviewDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        OverviewDockerPlugin(QObject *parent, const QVariantList &);
        ~OverviewDockerPlugin() override;
    private:
        KisViewManager* m_view;
};

#endif
