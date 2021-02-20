/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _PATTERN_DOCKER_H_
#define _PATTERN_DOCKER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class PatternDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        PatternDockerPlugin(QObject *parent, const QVariantList &);
        ~PatternDockerPlugin() override;
    private:
        KisViewManager* m_view;
};

#endif
