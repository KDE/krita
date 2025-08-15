/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOZOOMSTATE_H
#define KOZOOMSTATE_H

#include "KoZoomMode.h"
#include <boost/operators.hpp>
#include <QtGlobal>

/**
 * A simple structure to represent the zoom state.
 */
class KRITAFLAKE_EXPORT KoZoomState : public boost::equality_comparable<KoZoomState>
{
public:
    KoZoomState() = default;
    KoZoomState(KoZoomMode::Mode mode, qreal zoom, qreal minZoom, qreal maxZoom)
        : mode(mode), zoom(zoom), minZoom(minZoom), maxZoom(maxZoom) {}

    KoZoomMode::Mode mode = KoZoomMode::ZOOM_CONSTANT;
    qreal zoom {1.0};
    qreal minZoom {0.1};
    qreal maxZoom {90.0};

    bool operator==(const KoZoomState &other) const;
};

#endif // KOZOOMSTATE_H