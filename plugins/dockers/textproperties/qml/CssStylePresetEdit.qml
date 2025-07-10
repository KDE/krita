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
    property alias presetDescription: presetDescriptionArea.text;
    property alias presetSample: presetSampleLabel.svgData;
    property alias presetSampleAlignment: presetSampleLabel.alignment;
    property string styleType: "character"; // Defaults to character because the first index in the style combobox is character.
    property double canvasDPI: 72.0;

    property alias beforeSample: beforeSampleField.text;
    property alias sampleText: sampleField.text;
    property alias afterSample: afterSampleField.text;

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
        GridLayout {
            Layout.fillWidth: true;
            Layout.preferredHeight: implicitHeight;
            columns: 2;

            KoShapeQtQuickLabel {
                id: presetSampleLabel;
                Layout.fillWidth: true;
                Layout.columnSpan: 2;
                foregroundColor: palette.text;
                fullColor: false;

                scalingType: styleType === "paragraph"? KoShapeQtQuickLabel.FitWidth: KoShapeQtQuickLabel.Fit;
                Layout.preferredHeight: presetTitleField.height*4;
                padding: presetTitleField.height;

                Frame {
                    anchors.fill: parent;
                }
            }

            Label {
                Layout.minimumWidth: implicitWidth;
                text: i18nc("label:title", "Title:");
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter;
                horizontalAlignment: Text.AlignRight;
            }
            TextField {
                id: presetTitleField;
                Layout.fillWidth: true;
                placeholderText: i18nc("info:placeholder", "Style Name");
            }
            Label {
                Layout.minimumWidth: implicitWidth;
                text: i18nc("label:title", "Description:");
                Layout.alignment: Qt.AlignRight | Qt.AlignTop;
                horizontalAlignment: Text.AlignRight;
            }
            ScrollView {
                id: descriptionScroll
                Layout.fillWidth: true;
                Layout.preferredHeight: 3 * presetTitleField.height;

                TextArea {
                    id: presetDescriptionArea;
                    wrapMode: TextInput.Wrap;

                    placeholderText: i18nc("info:placeholder", "Style Description.");
                }

                background: Rectangle {
                    border.color: presetDescriptionArea.activeFocus? palette.highlight: palette.mid;
                    border.width: 1;
                    color: palette.base;
                }
            }
            Label {
                Layout.minimumWidth: implicitWidth;
                text: i18nc("label:Title", "Style Type:");
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter;
                horizontalAlignment: Text.AlignRight;
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
            Label {
                text: i18nc("label:Title", "Sample Text:");
            }
            RowLayout {
                Layout.columnSpan: 2;
                Layout.fillWidth: true;
                TextField {
                    id: beforeSampleField;
                    Layout.fillWidth: visible? true: false;
                    Layout.preferredWidth: visible? implicitWidth: 0;
                    placeholderText: i18nc("info:placeholder", "Before...");
                    onFocusChanged: if (!activeFocus) mainWindow.slotUpdateTextProperties();
                    visible: styleType !== "paragraph";
                }
                TextField {
                    id: sampleField;
                    Layout.fillWidth: true;
                    placeholderText: presetTitleField.text.length == 0?  i18nc("info:placeholder", "Sample..."): presetTitleField.text;
                    onFocusChanged: if (!activeFocus) mainWindow.slotUpdateTextProperties();
                }
                TextField {
                    id: afterSampleField;
                    Layout.fillWidth: visible? true: false;
                    Layout.preferredWidth: visible? implicitWidth: 0;
                    placeholderText: i18nc("info:placeholder", "... After");
                    onFocusChanged: if (!activeFocus) mainWindow.slotUpdateTextProperties();
                    visible: styleType !== "paragraph";
                }
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
