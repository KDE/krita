/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KOSELECTIONPRIVATE_H
#define KOSELECTIONPRIVATE_H

#include <QSharedData>

#include "kis_thread_safe_signal_compressor.h"

class KoShapeGroup;

class KoSelection::Private : public QSharedData
{
public:
    explicit Private()
        : QSharedData()
        , activeLayer(0)
        , selectionChangedCompressor(1, KisSignalCompressor::FIRST_INACTIVE)
    {}
    explicit Private(const Private &)
        : QSharedData()
        , activeLayer(0)
        , selectionChangedCompressor(1, KisSignalCompressor::FIRST_INACTIVE)
    {
    }
    QList<KoShape*> selectedShapes;
    KoShapeLayer *activeLayer;

    KisThreadSafeSignalCompressor selectionChangedCompressor;
};

#endif
