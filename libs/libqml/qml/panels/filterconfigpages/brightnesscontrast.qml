/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Item {
    id: base
    property QtObject configuration;
    function applyConfigurationChanges() {
        fullFilters.applyConfiguration(configuration);
    }
    function setProp(name, value) {
        if (configuration !== null) {
            configuration.writeProperty(name, value);
            base.applyConfigurationChanges();
        }
    }
    function setCurve(curve) {
        if (configuration !== null) {
            configuration.setCurve(curve);
            base.applyConfigurationChanges();
        }
    }
    onConfigurationChanged: {
        contrastCurve.setCurve(configuration.curve());
    }
    Column {
        anchors.fill: parent;
        Item {
            width: parent.width;
            height: Constants.DefaultMargin
        }
        Label {
            anchors.leftMargin: Constants.DefaultMargin;
            text: "Contrast curve:"
        }
        CurveEditorItem {
            id: contrastCurve
            width: parent.width;
            height: width;
            onCurveChanged: base.setCurve(curve);
            Button {
                height: Constants.DefaultFontSize + Constants.DefaultMargin * 2;
                width: Constants.DefaultFontSize + Constants.DefaultMargin * 2;
                anchors {
                    top: parent.top;
                    left: parent.left;
                }
                text: "-"
                onClicked: parent.deleteSelectedPoint();
                opacity: parent.pointSelected ? 1 : 0;
                Behavior on opacity { PropertyAnimation { duration: Constants.AnimationDuration; } }
            }
        }
    }
}
