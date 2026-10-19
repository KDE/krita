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

class KoSelection::Private
{
public:
    class LayerShapeGeometryProtectedListener : public KoShape::ShapeChangeListener
    {
    public:
        LayerShapeGeometryProtectedListener(KoSelection *parent)
            : m_parent(parent)
        {
        }

        void notifyShapeChanged(KoShape::ChangeType type, KoShape *shape) override;

    private:
        KoSelection *m_parent;
    };
public:
    explicit Private(KoSelection *parent)
        : activeLayer(0)
        , selectionChangedCompressor(new KisThreadSafeSignalCompressor(100, KisSignalCompressor::FIRST_INACTIVE))
        , activeLayerStateListener(parent)
    {}
    explicit Private(const Private &, KoSelection *parent)
        : activeLayer(0)
        , selectionChangedCompressor(new KisThreadSafeSignalCompressor(100, KisSignalCompressor::FIRST_INACTIVE))
        , activeLayerStateListener(parent)
    {
    }

    ~Private() {
        selectionChangedCompressor->deleteLater();
    }

    QList<KoShape*> selectedShapes;
    KoShapeLayer *activeLayer;

    KisThreadSafeSignalCompressor *selectionChangedCompressor;

    LayerShapeGeometryProtectedListener activeLayerStateListener;
};

#endif
