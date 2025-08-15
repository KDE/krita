/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCanvasState.h"
#include "kis_coordinates_converter.h"
#include "KoZoomState.h"

bool KisCanvasState::operator==(const KisCanvasState &other) const {
    return qFuzzyCompare(zoom, other.zoom) &&
           qFuzzyCompare(effectiveZoom, other.effectiveZoom) &&
           zoomMode == other.zoomMode &&
           qFuzzyCompare(rotation, other.rotation) &&
           mirrorHorizontally == other.mirrorHorizontally &&
           mirrorVertically == other.mirrorVertically &&
           documentOffset == other.documentOffset &&
           documentOffsetF == other.documentOffsetF &&
           viewportOffsetF == other.viewportOffsetF &&
           minimumOffset == other.minimumOffset &&
           maximumOffset == other.maximumOffset &&
           canvasSize == other.canvasSize &&
           qFuzzyCompare(minimumZoom, other.minimumZoom) &&
           qFuzzyCompare(maximumZoom, other.maximumZoom) &&
           imageRectInWidgetPixels == other.imageRectInWidgetPixels;
}

KisCanvasState KisCanvasState::fromConverter(const KisCoordinatesConverter &converter) {
    KisCanvasState state;
    state.zoom = converter.zoom();
    state.effectiveZoom = converter.effectiveZoom();
    state.zoomMode = converter.zoomMode();
    state.rotation = converter.rotationAngle();
    state.mirrorHorizontally = converter.xAxisMirrored();
    state.mirrorVertically = converter.yAxisMirrored();
    state.documentOffset = converter.documentOffset();
    state.documentOffsetF = converter.documentOffsetF();
    state.viewportOffsetF = converter.imageRectInViewportPixels().topLeft();
    state.minimumOffset = converter.minimumOffset();
    state.maximumOffset = converter.maximumOffset();
    state.canvasSize = converter.getCanvasWidgetSize();
    state.minimumZoom = converter.minZoom();
    state.maximumZoom = converter.maxZoom();
    state.imageRectInWidgetPixels = converter.imageRectInWidgetPixels();

    if (state.imageRectInWidgetPixels.topLeft() != -state.documentOffsetF) {
        qWarning() << "The imageRectInWidgetPixels topLeft() does not match the documentOffsetF!";
        qWarning() << "    imageRectInWidgetPixels:" << state.imageRectInWidgetPixels;
        qWarning() << "    documentOffsetF:" << state.documentOffsetF;
    }

    return state;
}

KoZoomState KisCanvasState::zoomState() const {
    KoZoomState state;
    state.mode = zoomMode;
    state.zoom = zoom;
    state.minZoom = minimumZoom;
    state.maxZoom = maximumZoom;
    return state;
}
