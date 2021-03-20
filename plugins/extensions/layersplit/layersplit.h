/*
 * SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LAYERSPLIT_H
#define LAYERSPLIT_H

#include <QVariant>

#include <QUrl>
#include <KisActionPlugin.h>

class LayerSplit : public KisActionPlugin
{
    Q_OBJECT
public:
    LayerSplit(QObject *parent, const QVariantList &);
    ~LayerSplit() override;

private Q_SLOTS:

    void slotLayerSplit();

};

#endif // LAYERSPLIT_H
