/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.krita.flake.text 1.0;
import org.krita.components 1.0 as Kis

Control {
    id: styleEdit;

    property TextPropertyConfigModel configModel: TextPropertyConfigModel {};
    property alias presetTitle: presetTitleField.text;
    property alias presetDescription: presetDescriptionArea.text;
    property alias presetSample: presetSampleLabel.svgData;
    property alias presetSampleAlignment: presetSampleLabel.alignment;
    property string styleType: "character"; // Defaults to character because the first index in the style combobox is character.
    property double canvasDPI: 72.0;
    property double pixelRelativeDPI: 72.0;
    property alias makePixelRelative: pixelRelativeCheck.checked;

    property var locales: [];
    property KoSvgTextPropertiesModel textProperties: KoSvgTextPropertiesModel{};


    onMakePixelRelativeChanged: mainWindow.slotUpdateStoreDPI();

    property alias beforeSample: beforeSampleField.text;
    property alias sampleText: sampleField.text;
    property alias afterSample: afterSampleField.text;

    onStyleTypeChanged: {
        styleTypeCmb.currentIndex = styleTypeCmb.indexOfValue(styleType === "paragraph"? TextPropertyConfigModel.Paragraph: TextPropertyConfigModel.Character);
    }

    Kis.ThemedControl {
        id: pal;
    }
    palette: pal.palette;

    background: Rectangle {
        color: palette.window;
    }

    GridLayout {
        anchors.fill: parent;
        GridLayout {
            Layout.minimumWidth: 250;
            Layout.maximumWidth: 300;
            Layout.fillHeight: true;
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
                text: i18nc("@label:textfield", "Title:");
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter;
                horizontalAlignment: Text.AlignRight;
            }
            TextField {
                id: presetTitleField;
                Layout.fillWidth: true;
                placeholderText: i18nc("@info:placeholder", "Style Name:");
                onTextChanged: mainWindow.slotUpdateDirty();
            }
            Label {
                Layout.minimumWidth: implicitWidth;
                text: i18nc("@label:textarea", "Description:");
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

                    placeholderText: i18nc("@info:placeholder", "Style Description.");
                    onTextChanged: mainWindow.slotUpdateDirty();
                }

                background: Rectangle {
                    border.color: presetDescriptionArea.activeFocus? palette.highlight: palette.mid;
                    border.width: 1;
                    color: palette.base;
                }
            }
            Label {
                Layout.minimumWidth: implicitWidth;
                text: i18nc("label:listbox", "Style Type:");
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter;
                horizontalAlignment: Text.AlignRight;
            }
            ComboBox {
                id: styleTypeCmb;
                Layout.fillWidth: true;
                model: [
                    {text: i18nc("@label:inlistbox", "Character"), value: TextPropertyConfigModel.Character},
                    {text: i18nc("@label:inlistbox", "Paragraph"), value: TextPropertyConfigModel.Paragraph}
                ];
                valueRole: "value";
                textRole: "text";
                onActivated: styleEdit.styleType = currentValue === TextPropertyConfigModel.Character? "character": "paragraph";
            }
            Item {
                width: 1;
                height: 1;
            }
            CheckBox {
                id: pixelRelativeCheck;
                text: i18nc("@option:check", "Style is Pixel Relative");
                ToolTip.text: i18nc("info:tooltip", "By default, styles are stored in points, meaning that the sizes in pixels will be different depending on document print resolution. Toggle this to ensure the style will maintain the same pixel size across different documents.");
                ToolTip.visible: hovered;
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
                hoverEnabled: true;
            }

            Label {
                text: i18nc("@label:textfield", "Sample Text:");
            }
            RowLayout {
                Layout.columnSpan: 2;
                Layout.fillWidth: true;
                TextField {
                    id: beforeSampleField;
                    Layout.fillWidth: visible? true: false;
                    Layout.preferredWidth: visible? implicitWidth: 0;
                    placeholderText: i18nc("@info:placeholder", "Before...");
                    onTextChanged: mainWindow.slotUpdateTextProperties();
                    visible: styleType !== "paragraph";
                }
                TextField {
                    id: sampleField;
                    Layout.fillWidth: true;
                    placeholderText: presetTitleField.text.length == 0?  i18nc("@info:placeholder", "Sample..."): presetTitleField.text;
                    onTextChanged: mainWindow.slotUpdateTextProperties();
                }
                TextField {
                    id: afterSampleField;
                    Layout.fillWidth: visible? true: false;
                    Layout.preferredWidth: visible? implicitWidth: 0;
                    placeholderText: i18nc("@info:placeholder", "... After");
                    onTextChanged: mainWindow.slotUpdateTextProperties();
                    visible: styleType !== "paragraph";
                }
            }
            Item {
                Layout.fillHeight: true;
                Layout.fillWidth: true;
            }
        }
        TextPropertyBaseList {
            propertiesModel: styleEdit.textProperties;
            locales: styleEdit.locales;
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            canvasDPI: styleEdit.makePixelRelative? styleEdit.pixelRelativeDPI: styleEdit.canvasDPI;
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
