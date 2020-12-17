/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_CANVAS_H
#define LIBKIS_CANVAS_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

class KoCanvasBase;
class KisDisplayColorConverter;

/**
 * Canvas wraps the canvas inside a view on an image/document.
 * It is responsible for the view parameters of the document:
 * zoom, rotation, mirror, wraparound and instant preview.
 */
class KRITALIBKIS_EXPORT Canvas : public QObject
{
    Q_OBJECT

public:
    explicit Canvas(KoCanvasBase *canvas, QObject *parent = 0);
    ~Canvas() override;

    bool operator==(const Canvas &other) const;
    bool operator!=(const Canvas &other) const;

public Q_SLOTS:

    /**
     * @return the current zoomlevel. 1.0 is 100%.
     */
    qreal zoomLevel() const;

    /**
     * @brief setZoomLevel set the zoomlevel to the given @p value. 1.0 is 100%.
     */
    void setZoomLevel(qreal value);

    /**
     * @brief resetZoom set the zoomlevel to 100%
     */
    void resetZoom();

    /**
     * @return the rotation of the canvas in degrees.
     */
    qreal rotation() const;

    /**
     * @brief setRotation set the rotation of the canvas to the given  @param angle in degrees.
     */
    void setRotation(qreal angle);

    /**
     * @brief resetRotation reset the canvas rotation.
     */
    void resetRotation();

    /**
     * @return return true if the canvas is mirrored, false otherwise.
     */
    bool mirror() const;

    /**
     * @brief setMirror turn the canvas mirroring on or off depending on @param value
     */
    void setMirror(bool value);

    /**
     * @return true if the canvas is in wraparound mode, false if not. Only when OpenGL is enabled,
     * is wraparound mode available.
     */
    bool wrapAroundMode() const;

    /**
     * @brief setWrapAroundMode set wraparound mode to  @param enable
     */
    void setWrapAroundMode(bool enable);

    /**
     * @return true if the canvas is in Instant Preview mode, false if not. Only when OpenGL is enabled,
     * is Instant Preview mode available.
     */
    bool levelOfDetailMode() const;

    /**
     * @brief setLevelOfDetailMode sets Instant Preview to @param enable
     */
    void setLevelOfDetailMode(bool enable);

    /**
     * @return the view that holds this canvas
     */
    View *view() const;


private:

    friend class ManagedColor;

    KisDisplayColorConverter *displayColorConverter() const;

    struct Private;
    Private *const d;

};

#endif // LIBKIS_CANVAS_H
