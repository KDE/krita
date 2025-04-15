/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.flake.text 1.0

TextPropertyBase {
    propertyName: i18nc("@label:spinbox", "White Space");
    toolTip: i18nc("@info:tooltip",
                   "The CSS white space rule controls how multiples of spaces are handled, and whether the text can wrap.");
    searchTerms: i18nc("comma separated search terms for the white-space property, matching is case-insensitive",
                       "white-space, collapse, pre-formatted");
    propertyType: TextPropertyBase.Mixed;
    property int whiteSpace: 0;

    readonly property int wsNormal: 0;
    readonly property int wsPre: 1;
    readonly property int wsNoWrap: 2;
    readonly property int wsPreWrap: 3;
    readonly property int wsBreakSpace: 4;
    readonly property int wsPreLine: 5;

    onPropertiesUpdated: {
        blockSignals = true;
        if (properties.textWrap !== KoSvgText.NoWrap) {
            if (properties.textCollapse === KoSvgText.Preserve) {
                whiteSpace = wsPreWrap;
            } else if (properties.textCollapse === KoSvgText.PreserveBreaks) {
                whiteSpace = wsPreLine;
            } else {
                whiteSpace = wsNormal;
            }
        } else {
            if (properties.textCollapse === KoSvgText.Preserve) {
                whiteSpace = wsPre;
            } else {
                whiteSpace = wsNoWrap;
            }
        }
        visible = properties.textCollapseState !== KoSvgTextPropertiesModel.PropertyUnset
        blockSignals = false;
    }

    onWhiteSpaceChanged: {
        whiteSpaceCmb.currentIndex = whiteSpaceCmb.indexOfValue(whiteSpace);
        if (!blockSignals) {
            if (whiteSpace === wsNormal) {
                properties.textCollapse = KoSvgText.Collapse;
                properties.textWrap = KoSvgText.Wrap;
            } else if (whiteSpace === wsPre) {
                properties.textCollapse = KoSvgText.Preserve;
                properties.textWrap = KoSvgText.NoWrap;
            } else if (whiteSpace === wsNoWrap) {
                properties.textCollapse = KoSvgText.Collapse;
                properties.textWrap = KoSvgText.NoWrap;
            } else if (whiteSpace === wsPreWrap) {
                properties.textCollapse = KoSvgText.Preserve;
                properties.textWrap = KoSvgText.Wrap;
            } else if (whiteSpace === wsBreakSpace) {
                properties.textCollapse = KoSvgText.PreserveBreaks;
                properties.textWrap = KoSvgText.Wrap;
            } else if (whiteSpace === wsPreLine) {
                properties.textCollapse = KoSvgText.PreserveBreaks;
                properties.textWrap = KoSvgText.Wrap;
            }
        }
    }

    onEnableProperty: properties.textCollapseState = KoSvgTextPropertiesModel.PropertySet;
    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.textCollapseState;
            onClicked: properties.textCollapseState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.maximumWidth: implicitWidth;
            font.italic: properties.textCollapseState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        ComboBox {
            id: whiteSpaceCmb
            model: [
                {text:i18nc("@label:inlistbox", "Normal"), value: wsNormal},
                {text:i18nc("@label:inlistbox", "Pre"), value: wsPre},
                {text:i18nc("@label:inlistbox", "No Wrap"), value: wsNoWrap},
                {text:i18nc("@label:inlistbox", "Pre Wrap"), value: wsPreWrap},
                {text:i18nc("@label:inlistbox", "Pre Line"), value: wsPreLine}
            ]
            Layout.fillWidth: true;
            onActivated: whiteSpace = currentValue;
            textRole: "text";
            valueRole: "value";
            wheelEnabled: true;
        }
    }
}
