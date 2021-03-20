/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef IMAGEENHANCEMENT_H
#define IMAGEENHANCEMENT_H

#include <QObject>
#include <QVariant>

class KritaImageEnhancement : public QObject
{
    Q_OBJECT
public:
    KritaImageEnhancement(QObject *parent, const QVariantList &);
    ~KritaImageEnhancement() override;
};

#endif
