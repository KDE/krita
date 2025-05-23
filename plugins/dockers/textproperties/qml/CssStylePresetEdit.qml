/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.krita.flake.text 1.0;

Control {
    id: styleEdit;

    property alias presetTitle: presetTitleField.text;
    property string styleType: "paragraph";
    onStyleTypeChanged: {
        styleTypeCmb.currentIndex = styleTypeCmb.indexOfValue(styleType === "paragraph"? TextPropertyBase.Paragraph: TextPropertyBase.Character);
    }

    PaletteControl {
        id: styleEditPalette;
    }
    SystemPalette {
        id: sysPalette;
        colorGroup: styleEditPalette.colorGroup;
    }
    palette: styleEditPalette.palette;

    background: Rectangle {
        color: palette.window;
    }

    function setProperties() {
        presetProperties.updateProperties();
    }

    ColumnLayout {
        anchors.fill: parent;
        RowLayout {
            Layout.fillWidth: true;
            Layout.preferredHeight: implicitHeight;
            Label {
                Layout.minimumWidth: implicitWidth;
                text: i18nc("label:Title", "Title:");
            }
            TextField {
                id: presetTitleField;
                Layout.fillWidth: true;
            }
        }
        RowLayout {
            Layout.fillWidth: true;
            Layout.preferredHeight: implicitHeight;
            Label {
                Layout.minimumWidth: implicitWidth;
                text: i18nc("label:Title", "Style Type:");
            }
            ComboBox {
                id: styleTypeCmb;
                Layout.fillWidth: true;
                model: [
                    {text: i18nc("listbox", "Character"), value: TextPropertyBase.Character},
                    {text: i18nc("listbox", "Paragraph"), value: TextPropertyBase.Paragraph}
                ];
                valueRole: "value";
                textRole: "text";
                onActivated: styleEdit.styleType = currentValue === TextPropertyBase.Character? "character": "paragraph";
            }
        }
        TextPropertyBaseList {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            id: presetProperties;
            propertyType: styleType == "character"? TextPropertyBase.Character: TextPropertyBase.Paragraph;
            onPropertyTypeChanged: {
                fillPropertyList();
                updateProperties();
            }
        }
    }
}
