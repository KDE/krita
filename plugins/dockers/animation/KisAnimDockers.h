/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _ANIMATION_DOCKERS_H_
#define _ANIMATION_DOCKERS_H_

#include <QObject>
#include <QVariantList>

class KisViewManager;

class AnimationDockersPlugin : public QObject
{
    Q_OBJECT
    public:
        AnimationDockersPlugin(QObject *parent, const QVariantList &);
        ~AnimationDockersPlugin() override;
    private:
        KisViewManager* m_view;
};

#endif
