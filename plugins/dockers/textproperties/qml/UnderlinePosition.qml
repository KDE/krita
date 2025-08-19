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
    propertyTitle: i18nc("@label:spinbox", "Underline Position");
    propertyName: "decoration-position";
    toolTip: i18nc("@info:tooltip",
                   "Specify the position of the underline for text-decoration.");
    searchTerms: i18nc("comma separated search terms for the text-decoration-position property, matching is case-insensitive",
                       "text-decoration-position, underline, left, right, under");
    propertyType: TextPropertyConfigModel.Mixed;

    property int horizontalPos: 0;
    property int verticalPos: 0;

    Connections {
        target: properties;
        function onTextDecorationUnderlinePosHorizontalChanged() {
            updateUnderlinePos();
            updateVisibility();
        }
        function onTextDecorationUnderlinePosVerticalChanged() {
            updateUnderlinePos();
            updateVisibility();
        }

        function onTextDecorationUnderlinePositionStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateUnderlinePos();
        updateVisibility();
    }

    function updateUnderlinePos() {
        blockSignals = true;
        horizontalPos = properties.textDecorationUnderlinePosHorizontal;
        verticalPos = properties.textDecorationUnderlinePosVertical;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.textDecorationUnderlinePositionState];
        setVisibleFromProperty();
    }

    onHorizontalPosChanged: {
        if (!blockSignals) {
            properties.textDecorationUnderlinePosHorizontal = horizontalPos;
        }
    }

    onVerticalPosChanged: {
        if (!blockSignals) {
            properties.textDecorationUnderlinePosVertical = verticalPos;
        }
    }

    onEnableProperty: properties.textDecorationUnderlinePositionState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.textDecorationUnderlinePositionState;
            onClicked: properties.textDecorationUnderlinePositionState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.textDecorationUnderlinePositionState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        ComboBox {
            model: [
                {text:i18nc("@label:inlistbox", "Auto"), value: KoSvgText.UnderlineAuto},
                {text:i18nc("@label:inlistbox", "Bottom"), value: KoSvgText.UnderlineUnder}
            ]
            id: horizontalPositionCmb;
            Layout.fillWidth: true
            textRole: "text";
            valueRole: "value";
            onActivated: horizontalPos = currentValue;
            wheelEnabled: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        ComboBox {
            model: [
                {text:i18nc("@label:inlistbox", "Vertical Left"), value: KoSvgText.UnderlineLeft},
                {text:i18nc("@label:inlistbox", "Vertical Right"), value: KoSvgText.UnderlineRight}
            ]
            id: verticalPositionCmb;
            Layout.fillWidth: true
            textRole: "text";
            valueRole: "value";
            onActivated: verticalPos = currentValue;
            wheelEnabled: true;
        }


    }
}
