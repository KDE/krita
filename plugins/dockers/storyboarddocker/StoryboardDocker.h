/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef STORYBOARDDOCKER_H
#define STORYBOARDDOCKER_H

#include <QObject>
#include <QVariant>


/**
 * Storyboard Docker showing frames and comments
 */
class StoryboardDockerPlugin : public QObject
{
    Q_OBJECT
public:
    StoryboardDockerPlugin(QObject *parent, const QVariantList &);
    ~StoryboardDockerPlugin() override;
};

#endif