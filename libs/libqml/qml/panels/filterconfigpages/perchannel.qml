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
    height: childrenRect.height;
    function applyConfigurationChanges() {
        fullFilters.applyConfiguration(configuration);
    }
    function setProp(name, value) {
        if (configuration !== null) {
            configuration.writeProperty(name, value);
            base.applyConfigurationChanges();
        }
    }
    function setCurve(index, curve) {
        if (configuration !== null) {
            configuration.setSpecificCurve(index, curve);
            base.applyConfigurationChanges();
        }
    }
    onConfigurationChanged: {
        numberOfChannels = configuration.curveCount();
    }
    property int numberOfChannels: 0;
    Column {
        id: contentsColumn;
        width: Constants.GridWidth * 2 - Constants.DefaultMargin * 2;
        height: childrenRect.height;
        Repeater {
            model: base.numberOfChannels;
            delegate: Item {
                height: childrenRect.height;
                width: contentsColumn.width;
                Item {
                    id: curveSpacer;
                    width: parent.width;
                    height: Constants.DefaultMargin;
                }
                Label {
                    id: curveLabel;
                    anchors.top: curveSpacer.bottom;
                    width: contentsColumn.width;
                    height: font.pixelSize + Constants.DefaultMargin * 2;
                    text: base.configuration.specificCurveName(index);
                }
                CurveEditorItem {
                    id: curveEditor;
                    width: contentsColumn.width;
                    height: width;
                    anchors.top: curveLabel.bottom;
                    // this currently crashes. seems to potentially be threading related,
                    // and leaving it out in reality has no effect, so for now at least,
                    // leaving it in and just ignoring it. Leaving comment because crash
                    //curve: base.configuration.specificCurve(index);
                    Connections {
                        target: base
                        onConfigurationChanged: curveEditor.curve = base.configuration.specificCurve(index);
                    }
                    onCurveChanged: base.setCurve(index, curve);
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
                Item {
                    anchors.top: curveEditor.bottom;
                    width: parent.width;
                    height: Constants.DefaultMargin;
                }
            }
        }
    }
}
