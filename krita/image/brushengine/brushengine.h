/*
 *  Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
  @mainpage Brush Engine API

All brush engines in Krita are plugins. The classes in this directory define
the way brush engines are to be implemented. Brush engines are also known as paintops.

KisPaintInformation         Information about the painting stroke
KisPaintOp                  Base class for brush engines: paints dots, lines and beziers. A new
                            paintop is created for every stroke.
KisPaintOpFactory           Creates a paintop and a paintop widget
KisPaintOpPreset            A KoResource that defines a set of parameters for a paintop. A KisPaintopPreset
                            contains a KisPaintopSettings widget.
KisPaintOpRegistry          Contains the list of all brush engine plugins
KisPaintOpSettings          A KisPropertiesConfiguration subclass that stores the settings for a paintop
KisPaintOpSettingsWidget    A KisConfigWidget that knows how to display and edit the KisPaintopSettings

These classes are used in the Krita core to handle painting: a further set
of utility classes for brush engine implementators is available in
krita/plugins/paintops/libpaintop.


The flow of action is as follows:

<ul>

<li>The resource system loads all paintop presets

<li>The user selects a particular resource in the gui

<li>The system displays the appropriate widget in the gui and fills it
with the KisPaintopSettings::KisPropertiesConfiguration that defines
the resource. The user can edit the settings, but they are not
automatically saved

<li>On mousedown, the paintop factory creates a paintop with a clone
of the specified settings (for now, we also need to pass the widget to
the settings class, for historical reasons)

<li>When painting, KisPaintInformation is passed to the paintop

<li>The paintop paints the lines

<li>On mouse-up, the paintop is destroyed

</ul>

#ifndef BRUSHENGINE
#define BRUSHENGINE
// Let's keep kRaZy happy
#endif

