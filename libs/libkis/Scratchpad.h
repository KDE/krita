/*
 *  SPDX-FileCopyrightText: 2020 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_SCRATCHPAD_H
#define LIBKIS_SCRATCHPAD_H

#include <QObject>
#include <QColor>
#include <kis_types.h>
#include "kritalibkis_export.h"
#include "libkis.h"
#include "View.h"

class KoCanvasBase;
class Canvas; // This comes from Python. This would be maybe better
class KisView;

/**
 * @brief The Scratchpad class
 * A scratchpad is a type of blank canvas area that can be painted on 
 * with the normal painting devices
 *
 */
class KRITALIBKIS_EXPORT Scratchpad: public QWidget
{
    Q_OBJECT
public:
    Scratchpad(View *view, const QColor & defaultColor, QWidget *parent = 0);
    ~Scratchpad();

public Q_SLOTS:

    /**
     * @brief Clears out scratchpad with color specified set during setup
     */
    void clear();

    /**
     * @brief Fill the entire scratchpad with default color
     */
    void fillDefault();

    /**
     * @brief Fill the entire scratchpad with current gradient
     * @param gradientVectorStart is a QPoint to define origin of gradient
     * Set an empty QPoint() to use default scratchpad top-left
     * @param gradientVectorEnd is a QPoint to define end of gradient
     * set an empty QPoint() to use default scratchpad bottom-right
     * @param gradientShape define which gradient to apply, can be:
     * - "linear"
     * - "bilinear"
     * - "radial"
     * - "square"
     * - "conical"
     * - "conicalSymmetric"
     * - "spiral"
     * - "reverseSpiral"
     * - "polygonal"
     * @param gradientRepeat define how to repeat gradient, can be:
     * - "none"
     * - "alternate"
     * - "forwards"
     * @param reverseGradient a boolean to define if gradient is reversed or not
     * @param dither a boolean to define if gradient is dithered or not
     */
    void fillGradient(const QPoint &gradientVectorStart = QPoint(),
                      const QPoint &gradientVectorEnd = QPoint(),
                      const QString &gradientShape = "linear",
                      const QString &gradientRepeat = "none",
                      bool reverseGradient = false,
                      bool dither = false);

    /**
     * @brief Fill the entire scratchpad with current background color
     */
    void fillBackground();

    /**
     * @brief Fill the entire scratchpad with current foreground color
     */
    void fillForeground();

    /**
     * @brief Fill the entire scratchpad with a transparent color
     */
    void fillTransparent();

    /**
     * @brief Fill the entire scratchpad with current document projection content
     * @param fullContent when True, full document projection is loaded in scratchpad, otherwise only content matching scratchpad viewport is loaded
     */
    void fillDocument(bool fullContent = true);

    /**
     * @brief Fill the entire scratchpad with current layer content
     * @param fullContent when True, full layer content is loaded in scratchpad, otherwise only content matching scratchpad viewport is loaded
     */
    void fillLayer(bool fullContent = true);

    /**
     * @brief Fill the entire scratchpad with current pattern
     * @param transform is QTransform that let define pattern scale/rotation property
     */
    void fillPattern(QTransform transform = QTransform());

    /**
     * @brief Define default fill color for scratchpad
     * @param Color to fill the canvas with
     */
    void setFillColor(QColor color);

    /**
     * @brief Switches between a GUI controlling the current mode and when mouse clicks control mode
     * @param value Set to True allows GUI to control the mode with explicitly setting mode
     */
    void setModeManually(bool value);

    /**
     * @brief Manually set what mode scratchpad is in. Ignored if "setModeManually is set to false
     * @param modeName Available options are:
     * - "painting"
     * - "panning"
     * - "colorsampling"
     */
    void setMode(QString modeName);

    /**
     * @brief DEPRECATED -- USE setCanvasZoomLink() instead
     * Makes a connection between the zoom of the canvas and scratchpad area so they zoom in sync
     * @param value If True (default) the scratchpad will share the current view zoom level.
     * If False, then use scratchpad scale methods to define current zoom level
     */
    void linkCanvasZoom(bool value);

    /**
     * @brief return if scratchpad zoom level is linked with current view zoom level
     * @return return True if connection between the zoom of the canvas and scratchpad (so they zoom in sync) is active
     */
    bool canvasZoomLink();

    /**
     * @brief Makes a connection between the zoom of the canvas and scratchpad area so they zoom in sync
     * @param value If True (default) the scratchpad will share the current view zoom level.
     * If False, then use scratchpad scale methods to define current zoom level
     */
    void setCanvasZoomLink(bool value);

    /**
     * @brief return current zoom level applied on scratchpad (whatever the zoom source is: view zoom level or set manually)
     * @return a float value (1.00 = 100%)
     */
    qreal scale();

    /**
     * @brief allow to manually set scratchpad zoom level
     * Note: call method is ignored if canvasZoomLink() is True,
     * @param scale zoom level to apply (1.00 = 100%)
     * @return if scale has been applied return True, otherwise return False
     */
    bool setScale(qreal scale) const;

    /**
     * @brief calculate scale automatically to fit scratchpad content in scratchpad viewport
     * Note: call method is ignored if canvasZoomLink() is True
     */
    void scaleToFit();

    /**
     * @brief reset scale and pan to origin
     * Note: call method is ignored if canvasZoomLink() is True
     */
    void scaleReset();

    /**
     * @brief pan scratchpad content to top-left position of scratchpad viewport
     * Provided value are absolute
     * @param x abscissa position to pan to
     * @param y ordinate position to pan to
     */
    void panTo(qint32 x, qint32 y);

    /**
     * @brief pan scratchpad content to center content in viewport
     */
    void panCenter();

    /**
     * @brief Load image data to the scratchpad
     * @param image Image object to load
     */
    void loadScratchpadImage(QImage image);

    /**
     * @brief Take what is on the scratchpad area and grab image
     * @return the image data from the scratchpad
     */
    QImage copyScratchpadImageData();

    /**
     * @brief The viewport indicates which part of scratchpad content is visible.
     * It takes in account the current translation & scale
     *
     * Example 1:
     * - Scratchpad size: 500x500
     * - Scratchpad content: 2000x2000
     * - Scratchpad scale: 1.0
     * - Scratchpad pan:   0, 0
     * Returned viewport is a QRect(0, 0, 500, 500) matching content really visible in scratchpad.
     * If scale is 2.00, returned viewport will be QRect(0, 0, 250, 250)
     * If scale is 0.50, returned viewport will be QRect(0, 0, 1000, 1000)
     *
     * Example 2:
     * - Scratchpad size: 500x500
     * - Scratchpad content: 2000x2000
     * - Scratchpad scale: 2.0
     * - Scratchpad pan:   500, 1500
     * Returned viewport is a QRect(500, 1500, 250, 250) matching content really visible in scratchpad.
     *
     * @return scratchpad viewport bounds as a QRect
     */
    QRect viewportBounds() const;

    /**
     * @brief The content of scratchpad can be bigger or smaller than scratchpad dimension.
     * The bounds return the area in which there's some content
     * @return scratchpad content bounds as a QRect
     */
    QRect contentBounds() const;

Q_SIGNALS:
    /**
     * @brief signal is emitted when scratchpad scale is changed (from zoom canvas or manually)
     * @param scale updated scale value (1.00 = 100%)
     */
    void scaleChanged(qreal scale);

    /**
     * @brief signal is emitted when scratchpad content is changed (stroke or fill)
     */
    void contentChanged();

    /**
     * @brief signal is emitted when scratchpad viewport has been modified (pan, zoom)
     * @param rect new viewport bounds
     */
    void viewportChanged(const QRect rect);

private:
    struct Private;
    const QScopedPointer<Private> d;

};

#endif // LIBKIS_SCRATCHPAD_H

