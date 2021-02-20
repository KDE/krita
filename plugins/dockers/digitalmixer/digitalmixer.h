/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _DIGITALMIXER_H_
#define _DIGITALMIXER_H_

#include <QObject>
#include <QVariant>

class KisViewManager;

/**
 * Template of view plugin
 */
class DigitalMixerPlugin : public QObject
{
    Q_OBJECT
    public:
        DigitalMixerPlugin(QObject *parent, const QVariantList &);
        ~DigitalMixerPlugin() override;
    private:
        KisViewManager* m_view;
};

#endif
