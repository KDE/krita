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

    property TextPropertyConfigModel configModel: TextPropertyConfigModel {};
    property alias presetTitle: presetTitleField.text;
    property string styleType: "paragraph";
    property double canvasDPI: 72.0;
    onStyleTypeChanged: {
        styleTypeCmb.currentIndex = styleTypeCmb.indexOfValue(styleType === "paragraph"? TextPropertyConfigModel.Paragraph: TextPropertyConfigModel.Character);
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
                    {text: i18nc("listbox", "Character"), value: TextPropertyConfigModel.Character},
                    {text: i18nc("listbox", "Paragraph"), value: TextPropertyConfigModel.Paragraph}
                ];
                valueRole: "value";
                textRole: "text";
                onActivated: styleEdit.styleType = currentValue === TextPropertyConfigModel.Character? "character": "paragraph";
            }
        }
        TextPropertyBaseList {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            canvasDPI: styleEdit.canvasDPI;
            configModel: styleEdit.configModel;
            id: presetProperties;
            propertyType: styleType == "character"? TextPropertyConfigModel.Character: TextPropertyConfigModel.Paragraph;
            onPropertyTypeChanged: {
                fillPropertyList();
                updateProperties();
            }
        }
    }
}
