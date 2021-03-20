/*
 * imagesplit.h -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 * SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef IMAGESPLIT_H
#define IMAGESPLIT_H

#include <QVariant>

#include <QUrl>
#include <KisActionPlugin.h>

class Imagesplit : public KisActionPlugin
{
    Q_OBJECT
public:
    Imagesplit(QObject *parent, const QVariantList &);
    ~Imagesplit() override;

private Q_SLOTS:

    void slotImagesplit();
    bool saveAsImage(const QRect &imgSize, const QString &mimeType, const QString &url);
};

#endif // IMAGESPLIT_H
