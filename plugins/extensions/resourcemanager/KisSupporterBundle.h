/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISSUPPORTERBUNDLE_H_
#define __KISSUPPORTERBUNDLE_H_

#include <QHash>
#include <QPixmap>
#include <QSet>
#include <QString>

struct KisSupporterBundle {
    QString fileName;
    long long sizeInBytes;
    QString source;
    QString title;
    QString description;
    QString author;
    QString license;
    QString checksum;
    QPixmap thumbnail;
    QSet<QString> tags;
    QHash<QString, int> resourceCountByMediaType;
};

#endif
