/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _SPECIFICCOLORSELECTOR_H_
#define _SPECIFICCOLORSELECTOR_H_

#include <QObject>
#include <QVariant>


/**
 * Template of view plugin
 */
class SpecificColorSelectorPlugin : public QObject
{
    Q_OBJECT
public:
    SpecificColorSelectorPlugin(QObject *parent, const QVariantList &);
    ~SpecificColorSelectorPlugin() override;
};

#endif
