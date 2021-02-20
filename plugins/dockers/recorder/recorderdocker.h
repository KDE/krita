/*
 *  SPDX-FileCopyrightText: 2019 Shi Yan <billconan@gmail.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _RECORDER_DOCKER_H_
#define _RECORDER_DOCKER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class RecorderDockerPlugin : public QObject
{
    Q_OBJECT
public:
    RecorderDockerPlugin(QObject* parent, const QVariantList&);
    ~RecorderDockerPlugin() override;

private:
    KisViewManager* m_view;
};

#endif
