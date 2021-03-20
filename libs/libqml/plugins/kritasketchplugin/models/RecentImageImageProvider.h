/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef RECENTIMAGEIMAGEPROVIDER_H
#define RECENTIMAGEIMAGEPROVIDER_H

#include <QQuickImageProvider>

class RecentImageImageProvider : public QQuickImageProvider
{
public:
    explicit RecentImageImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // RECENTIMAGEIMAGEPROVIDER_H
