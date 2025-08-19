/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

TextPropertyBase {
    propertyTitle: i18nc("@label:listbox", "Dominant Baseline");
    propertyName: "dominant-baseline";
    propertyType: TextPropertyConfigModel.Mixed;
    toolTip: i18nc("@info:tooltip",
                   "Dominant Baseline specifies how stretches of text of different sizes are aligned, it is also the default for Alignment Baseline.");
    searchTerms: i18nc("comma separated search terms for the dominant-baseline property, matching is case-insensitive",
                       "dominant-baseline, mojisoroe");
    property int baselineSelection

    Connections {
        target: properties;
        function onDominantBaselineChanged() {
            updateDominantBaseline();
            updateVisibility();
        }

        function onDominantBaselineStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateDominantBaseline();
        updateVisibility();
    }
    function updateDominantBaseline() {
        blockSignals = true;
        baselineSelection = properties.dominantBaseline;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.dominantBaselineState];
        setVisibleFromProperty();
    }

    onBaselineSelectionChanged: {
        baselineCmb.currentIndex = baselineCmb.indexOfValue(baselineSelection);
        if (!blockSignals) {
            properties.dominantBaseline = baselineSelection;
        }
    }

    onEnableProperty: properties.dominantBaselineState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.dominantBaselineState;
            onClicked: properties.dominantBaselineState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.dominantBaselineState === KoSvgTextPropertiesModel.PropertyTriState;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        SqueezedComboBox {
            model: [
                {text: i18nc("@label:inlistbox", "Auto"), value: KoSvgText.BaselineAuto, icon: "",
                    toolTip: i18nc("@info:tooltip", "Aligns to alphabetic baseline when laying out horizontally, aligns to central baseline when laying out vertically.")},
                {text: i18nc("@label:inlistbox", "Alphabetic"), value: KoSvgText.BaselineAlphabetic, icon: "qrc:///baseline-alphabetic.svg",
                    toolTip: i18nc("@info:tooltip", "Align to the baseline for scripts that align at the bottom.")},
                {text: i18nc("@label:inlistbox", "Ideographic"), value: KoSvgText.BaselineIdeographic, icon: "qrc:///baseline-ideo-embox-bottom.svg",
                    toolTip: i18nc("@info:tooltip", "Align to the ideographic em box bottom.")},
                {text: i18nc("@label:inlistbox", "Central"), value: KoSvgText.BaselineCentral, icon: "qrc:///baseline-ideo-embox-horizontal-center.svg",
                    toolTip: i18nc("@info:tooltip", "Align to the ideographic em box center.")},
                {text: i18nc("@label:inlistbox", "Hanging"), value: KoSvgText.BaselineHanging, icon: "qrc:///baseline-hanging.svg",
                    toolTip: i18nc("@info:tooltip", "Align to the headstroke as used by North-Brahmic scripts.")},
                {text: i18nc("@label:inlistbox", "Middle"), value: KoSvgText.BaselineMiddle, icon: "qrc:///baseline-x-middle.svg",
                    toolTip: i18nc("@info:tooltip", "Align to the center between the alphabetic baseline and x-height when laying out horizontally, in vertical this is the central baseline.")},
                {text: i18nc("@label:inlistbox", "Mathematical"), value: KoSvgText.BaselineMathematical, icon: "qrc:///baseline-math.svg",
                    toolTip: i18nc("@info:tooltip", "Align to the mathematical baseline, so that operator symbols align.")},
                {text: i18nc("@label:inlistbox", "Text Top"), value: KoSvgText.BaselineTextTop, icon: "qrc:///baseline-text-top.svg",
                    toolTip: i18nc("@info:tooltip", "Align to the ascender.")},
                {text: i18nc("@label:inlistbox", "Text Bottom"), value: KoSvgText.BaselineTextBottom, icon: "qrc:///baseline-text-bottom.svg",
                    toolTip: i18nc("@info:tooltip", "Align to the descender.")}
            ]
            Layout.fillWidth: true;
            id: baselineCmb;
            textRole: "text";
            valueRole: "value";
            iconRole: "icon";
            iconSize: 16;
            toolTipRole: "toolTip";
            onActivated: baselineSelection = currentValue;
            wheelEnabled: true;
        }
    }
}
