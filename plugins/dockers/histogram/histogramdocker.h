/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _HISTOGRAM_DOCKER_H_
#define _HISTOGRAM_DOCKER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class HistogramDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        HistogramDockerPlugin(QObject *parent, const QVariantList &);
        ~HistogramDockerPlugin() override;
    private:
        KisViewManager* m_view;
};

#endif
