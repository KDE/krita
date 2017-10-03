/*
*   Copyright (C) 2010 by Artur Duque de Souza <asouzakde.org>
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import QtQuick 2.3

/**
 * A simple busy indicator,
 * It is used to indicate a task whose duration is unknown. If the task
 * duration/number of steps is known, a ProgressBar should be used instead.
 * 
 * This is essentially the Plasma QML component, just without the
 * Plasma (SVG)requirement, for smaller footprint for the time being.
 */
Item {
    id: busy

    /**
     * This property holds whether the busy animation is running.
     *
     * The default value is false.
     */
    property bool running: false

    // Plasma API
    /**
     * Set this property if you don't want to apply a filter to smooth
     * the busy icon while animating.
     * Smooth filtering gives better visual quality, but is slower.
     *
     * The default value is true.
     */
    property bool smoothAnimation: true

    implicitWidth: 52
    implicitHeight: 52

    // Should use animation's pause to keep the
    // rotation smooth when running changes but
    // it has lot's of side effects on
    // initialization.
    onRunningChanged: {
        rotationAnimation.from = rotation;
        rotationAnimation.to = rotation + 360;
    }

    RotationAnimation on rotation {
        id: rotationAnimation

        from: 0
        to: 360
        duration: 1500
        running: busy.running
        loops: Animation.Infinite
    }

    Image {
        id: widget
        source: Settings.theme.image("busyindicator.png");
        anchors.centerIn: parent
        width: Math.min(busy.width, busy.height)
        height: width
        fillMode: Image.PreserveAspectFit
        smooth: !running || smoothAnimation
    }
}
