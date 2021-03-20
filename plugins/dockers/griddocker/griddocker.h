/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _GRID_DOCKER_H_
#define _GRID_DOCKER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class GridDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        GridDockerPlugin(QObject *parent, const QVariantList &);
        ~GridDockerPlugin() override;
    private:
        KisViewManager* m_view;
};

#endif
