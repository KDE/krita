/*
 *  Copyright (c) 2020 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

