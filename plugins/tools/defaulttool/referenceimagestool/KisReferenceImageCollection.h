/*
 *  SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISREFERENCEIMAGECOLLECTION_H
#define KISREFERENCEIMAGECOLLECTION_H

#include <QVector>

class QIODevice;
class KisReferenceImage;

class KisReferenceImageCollection
{
public:
    explicit KisReferenceImageCollection() = default;
    explicit KisReferenceImageCollection(const QVector<KisReferenceImage*> &references);

    const QVector<KisReferenceImage*> &referenceImages() const;

    bool save(QIODevice *io);
    bool load(QIODevice *io);

private:
    QVector<KisReferenceImage*> references;
};

#endif
