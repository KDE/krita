/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef H_ARTISTIC_COLOR_SELECTOR_PLUGIN_H
#define H_ARTISTIC_COLOR_SELECTOR_PLUGIN_H

#include <QObject>
#include <QVariant>

class ArtisticColorSelectorPlugin: public QObject
{
public:
    ArtisticColorSelectorPlugin(QObject *parent, const QVariantList &);
};

#endif // H_ARTISTIC_COLOR_SELECTOR_PLUGIN_H
