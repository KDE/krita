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
    propertyTitle: i18nc("@label:listbox", "Alignment Baseline");
    propertyName: "alignment-baseline";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Alignment baseline allows controlling how this range of text is aligned to the parent text.");
    searchTerms: i18nc("comma separated search terms for the alignment-baseline property, matching is case-insensitive",
                       "alignment-baseline, mojisoroe");
    property int baselineSelection;

    Connections {
        target: properties;
        function onAlignmentBaselineChanged() {
            updateAlignmentBaseline();
            updateVisibility();
        }

        function onAlignmentBaselineStateChanged() {
            updateAlignmentBaseline();
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateAlignmentBaseline();
        updateVisibility();
    }
    function updateAlignmentBaseline() {
        blockSignals = true;
        baselineSelection = properties.alignmentBaseline;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.alignmentBaselineState];
        setVisibleFromProperty();
    }

    onBaselineSelectionChanged: {
        let idx = baselineCmb.indexOfValue(baselineSelection);
        baselineCmb.currentIndex = idx > 0? idx: 0;
        if (!blockSignals) {
            properties.alignmentBaseline = baselineSelection;
        }
    }

    onEnableProperty: properties.alignmentBaselineState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.alignmentBaselineState;
            onClicked: properties.alignmentBaselineState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.alignmentBaselineState === KoSvgTextPropertiesModel.PropertyTriState;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        SqueezedComboBox {
            model: [
                {text: i18nc("@label:inlistbox", "Baseline"), value: KoSvgText.BaselineDominant, icon: "",
                    toolTip: i18nc("@info:tooltip", "Align to the baseline defined by Dominant Baseline.")},
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
