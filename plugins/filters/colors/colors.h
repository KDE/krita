/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COLORS_H
#define COLORS_H

#include <QObject>
#include <QVariant>

class KritaExtensionsColors : public QObject
{
    Q_OBJECT
public:
    KritaExtensionsColors(QObject *parent, const QVariantList &);
    ~KritaExtensionsColors() override;
};

#endif
