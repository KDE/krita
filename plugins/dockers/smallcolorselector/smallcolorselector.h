/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _SMALLCOLORSELECTOR_H_
#define _SMALLCOLORSELECTOR_H_

#include <QObject>
#include <QVariant>


/**
 * Template of view plugin
 */
class SmallColorSelectorPlugin : public QObject
{
    Q_OBJECT
public:
    SmallColorSelectorPlugin(QObject *parent, const QVariantList &);
    ~SmallColorSelectorPlugin() override;
};

#endif
