/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoZoomState.h"

bool KoZoomState::operator==(const KoZoomState &other) const {
    return mode == other.mode &&
           qFuzzyCompare(zoom, other.zoom) &&
           qFuzzyCompare(minZoom, other.minZoom) &&
           qFuzzyCompare(maxZoom, other.maxZoom);
}