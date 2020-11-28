/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _COLORSELECTORNG_H_
#define _COLORSELECTORNG_H_

#include <QObject>
#include <QVariant>


/**
 * Template of view plugin
 */
class ColorSelectorNgPlugin : public QObject
{
    Q_OBJECT
public:
    ColorSelectorNgPlugin(QObject *parent, const QVariantList &);
    ~ColorSelectorNgPlugin() override;
};

#endif
