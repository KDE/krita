/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
