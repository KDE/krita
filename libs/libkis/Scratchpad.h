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
#include "kis_scratch_pad.h"
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
     * clears out scratchpad with color specfified set during setup
     */
    void clear();

    void setFillColor(QColor color);

    /** Switches between a GUI controlling the current mode and when mouse clicks control mode
     * Setting to true allows GUI to control the mode with explicity setting mode
     */
    void setModeManually(bool value);


    /// Manually set what mode scratchpad is in. Ignored if "setModeManually is set to false
    void setMode(QString modeName);

    /// Should the scratchpad share the zoom level with the canvas?
    void linkCanvasZoom(bool value);


    /// load scratchpad
    void loadScratchpadImage(QImage image);

    /// take what is on scratchpad area and grab image
    QImage copyScratchpadImageData();


private:
    struct Private;
    const QScopedPointer<Private> d;

};

#endif // LIBKIS_SCRATCHPAD_H

