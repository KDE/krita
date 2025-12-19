/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

TextPropertyBase {
    propertyTitle: i18nc("@title:group", "Text Decoration");
    propertyName: "text-decoration";
    propertyType: TextPropertyConfigModel.Character;
    toolTip: i18nc("@info:tooltip",
                   "Text decoration allows drawing underlines, overlines and striking through text.");
    searchTerms: i18nc("comma separated search terms for the text-decoration property, matching is case-insensitive",
                       "underline, overline, strikethrough, line-through");

    property alias underline: underlineCbx.checked;
    property alias overline: overlineCbx.checked;
    property alias linethrough: linethroughCbx.checked;
    property int lineStyle;
    property color lineColor;

    Connections {
        target: properties;
        function onTextDecorationLineThroughChanged() {
            updateTextDecorThrough();
            updateVisibility();
        }
        function onTextDecorationUnderlineChanged() {
            updateTextDecorUnder();
            updateVisibility();
        }
        function onTextDecorationOverlineChanged() {
            updateTextDecorOver();
            updateVisibility();
        }

        function onTextDecorationStyleChanged() {
            updateTextDecorStyle();
            updateVisibility();
        }

        function onTextDecorationColorChanged() {
            updateTextDecorColor();
            updateVisibility();
        }

        function onTextDecorationLineStateChanged() {
            updateVisibility();
        }
        function onTextDecorationStyleStateChanged() {
            updateVisibility();
        }
        function onTextDecorationColorStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateTextDecorThrough();
        updateTextDecorUnder();
        updateTextDecorOver();
        updateTextDecorStyle();
        updateTextDecorColor();
        updateVisibility();
    }

    function updateTextDecorOver() {
        blockSignals = true;
        overline = properties.textDecorationOverline;
        blockSignals = false;
    }
    function updateTextDecorThrough() {
        blockSignals = true;
        linethrough = properties.textDecorationLineThrough;
        blockSignals = false;
    }
    function updateTextDecorUnder() {
        blockSignals = true;
        underline = properties.textDecorationUnderline;
        blockSignals = false;
    }

    function updateTextDecorStyle() {
        blockSignals = true;
        lineStyle = properties.textDecorationStyle;
        blockSignals = false;
    }

    function updateTextDecorColor() {
        blockSignals = true;
        lineColor = properties.textDecorationColor;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [
            properties.textDecorationLineState,
            properties.textDecorationStyleState,
            properties.textDecorationColorState
        ];
        setVisibleFromProperty();
    }

    onLineStyleChanged: {
        lineStyleCmb.currentIndex = lineStyleCmb.indexOfValue(lineStyle);
        if (!blockSignals) {
            properties.textDecorationStyle = lineStyle;
        }
    }

    onUnderlineChanged: {
        if (!blockSignals) {
            properties.textDecorationUnderline = underline;
        }
    }

    onOverlineChanged: {
        if (!blockSignals) {
            properties.textDecorationOverline = overline;
        }
    }

    onLinethroughChanged: {
        if (!blockSignals) {
            properties.textDecorationLineThrough = linethrough;
        }
    }

    onLineColorChanged: {
        if (!blockSignals) {
            properties.textDecorationColor = lineColor;
        }
    }

    onEnableProperty: {
        properties.textDecorationLineState = KoSvgTextPropertiesModel.PropertySet;
    }

    GridLayout {
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.textDecorationLineState;
            inheritable: false;
            onClicked: properties.textDecorationLineState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            Layout.columnSpan: 2;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.textDecorationLineState === KoSvgTextPropertiesModel.PropertyTriState;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: i18nc("@title:group", "Line:")
            Layout.columnSpan: 2;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            Layout.columnSpan: 2;
            text: i18nc("@option:check", "Underline")
            icon.height: 16;
            icon.width: 16;
            icon.source: "qrc:///16_light_format-text-underline.svg"
            icon.color: palette.text;
            display: AbstractButton.TextBesideIcon;
            id: underlineCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            Layout.columnSpan: 2;
            text: i18nc("@option:check", "Overline")
            icon.height: 16;
            icon.width: 16;
            icon.source: "qrc:///16_light_format-text-overline.svg"
            icon.color: palette.text;
            display: AbstractButton.TextBesideIcon;
            id: overlineCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            Layout.columnSpan: 2;
            text: i18nc("@option:check", "Line-through")
            icon.height: 16;
            icon.width: 16;
            icon.source: "qrc:///16_light_format-text-strikethrough.svg"
            icon.color: palette.text;
            display: AbstractButton.TextBesideIcon;
            id: linethroughCbx;
        }


        RevertPropertyButton {
            revertState: properties.textDecorationColorState;
            inheritable: false;
            onClicked: properties.textDecorationColorState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: i18nc("@label:chooser", "Color:")
            id: colorCbx;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.textDecorationColorState === KoSvgTextPropertiesModel.PropertyTriState;
        }
        Button {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            contentItem: Rectangle {
                color: lineColor;
            }
            onClicked: {
                lineColor = mainWindow.modalColorDialog(lineColor);
            }
        }

        RevertPropertyButton {
            revertState: properties.textDecorationStyleState;
            inheritable: false;
            onClicked: properties.textDecorationStyleState = KoSvgTextPropertiesModel.PropertyUnset;
        }
        Label {
            text: i18nc("@label:listbox", "Style:")
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.textDecorationStyleState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        ComboBox {
            id: lineStyleCmb;
            model: [
                { text: i18nc("@label:inlistbox", "Solid"), value: KoSvgText.Solid},
                { text: i18nc("@label:inlistbox", "Dotted"), value: KoSvgText.Dotted},
                { text: i18nc("@label:inlistbox", "Dashed"), value: KoSvgText.Dashed},
                { text: i18nc("@label:inlistbox", "Double"), value: KoSvgText.Double},
                { text: i18nc("@label:inlistbox", "Wavy"), value: KoSvgText.Wavy},
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: lineStyle = currentValue;
            wheelEnabled: true;
        }
    }
}
