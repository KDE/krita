/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _ARRANGE_DOCKER_H_
#define _ARRANGE_DOCKER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class ArrangeDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        ArrangeDockerPlugin(QObject *parent, const QVariantList &);
        ~ArrangeDockerPlugin() override;
    private:
        KisViewManager* m_view;
};

#endif
