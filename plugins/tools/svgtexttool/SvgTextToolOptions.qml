/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.tools.text 1.0
import org.krita.components 1.0 as Kis

Control {
    id: toolOptionsControl;
    anchors.fill: parent;

    property SvgTextToolOptionsManager manager: SvgTextToolOptionsManager{};

    onManagerChanged: {
        updateTextProperties()
        textType.updateTextTypes();
        updatePresetName()
    }

    Connections {
        target: manager;
        function onTextPropertiesOpenChanged() {
            updateTextProperties();
        }
        function onTextTypeChanged() {
            textType.updateTextTypes();
        }
    }

    Connections {
        target: manager.optionsModel;
        function onUseVisualBidiCursorChanged() {
            chkVisualBidiCursor.checked = manager.optionsModel.useVisualBidiCursor;
        }
        function onPasteRichtTextByDefaultChanged() {
            chkPasteRichTextByDefault.checked = manager.optionsModel.pasteRichtTextByDefault;
        }
        function onCssStylePresetNameChanged() {
            updatePresetName();
        }
    }

    function updatePresetName() {
        btnSelectCssStylePreset.resourceName = manager.optionsModel.cssStylePresetName;
    }

    Kis.ThemedControl {
        id: themedControl;
    }
    palette: themedControl.palette;
    contentItem: ColumnLayout {
        id: childColumn;
        width: parent.width;

        Label {
            text: i18nc("@label:group", "Create New Texts with...");
            Layout.fillWidth: true;
        }

        CheckBox {
            id: chkCurrentProperties;
            text: i18nc("@option:check", "Current Text Properties");
            checked: manager.optionsModel.useCurrentTextProperties;
            onCheckedChanged: {
                manager.optionsModel.useCurrentTextProperties = checked;
            }
            Layout.fillWidth: true;
            hoverEnabled: true;
            Kis.ToolTipBase {
                parent: chkCurrentProperties;
                text: i18nc("@info:tooltip", "Use the current properties in the text properties docker to create new texts.");
                visible: parent.hovered;
            }
        }
        Kis.ResourcePopup {
            id: btnSelectCssStylePreset;
            Layout.fillWidth: true;
            enabled: !chkCurrentProperties.checked;
            useFileName: false;

            placeholderText: i18nc("@info:placeholder", "Select Style Preset")

            resourceType: "css_styles";
            resourceDelegate: Kis.CssStylePresetDelegate {
                id: presetDelegate;
                resourceView: btnSelectCssStylePreset.view;
                onResourceLeftClicked: {
                    btnSelectCssStylePreset.activated();
                }

            }
            view.preferredHeight: 256; /// roughly 3x preset delegate size;

            onResourceNameChanged: {
                manager.optionsModel.cssStylePresetName = btnSelectCssStylePreset.resourceName;
            }
        }

        Kis.ToolSeparatorBase {
            orientation: Qt.Horizontal;
            Layout.fillWidth: true;
        }

        Button {
            id: btnTextProperties;
            text: i18nc("@label:button", "Open Text Properties");
            checkable: false;
            hoverEnabled: true;
            onClicked: {
                manager.textPropertiesOpen = !manager.textPropertiesOpen;
            }
            visible: manager.showTextPropertyButton;
            height: visible? implicitHeight: 0;
            Layout.fillWidth: true;

            Kis.ToolTipBase {
                parent: btnTextProperties;
                text: i18nc("@info:tooltip", "Open the Text Properties Docker to edit properties like font size and style.");
                visible: parent.hovered;
            }
        }
        Kis.ToolSeparatorBase {
            orientation: Qt.Horizontal;
            Layout.fillWidth: true;
        }

        Label {
            text: i18nc("@label:group", "Options");
            Layout.fillWidth: true;
        }

        CheckBox {
            id: chkVisualBidiCursor;
            text: i18nc("@option:check", "Use Visual Cursor");
            checked: manager.optionsModel.useVisualBidiCursor;
            onCheckedChanged: {
                manager.optionsModel.useVisualBidiCursor = checked;
            }
            Kis.ToolTipBase {
                parent: chkVisualBidiCursor;
                text: i18nc("@info:tooltip", "Use the visual order for bidirectional text.");
                visible: parent.hovered;
            }
            hoverEnabled: true;
            Layout.fillWidth: true;
        }

        CheckBox {
            id: chkPasteRichTextByDefault;
            text: i18nc("@option:check", "Paste Rich Text by Default");
            checked: manager.optionsModel.pasteRichtTextByDefault;
            onCheckedChanged: {
                manager.optionsModel.pasteRichtTextByDefault = checked;
            }
            Kis.ToolTipBase {
                parent: chkPasteRichTextByDefault;
                text: i18nc("@info:tooltip", "Always paste rich text when using paste action.");
                visible: parent.hovered;
            }
            hoverEnabled: true;
            Layout.fillWidth: true;
        }

        Kis.ToolSeparatorBase {
            orientation: Qt.Horizontal;
            Layout.fillWidth: true;
        }

        Label {
            text: i18nc("@label:group", "Dialogs");
        }

        Button {
            id: btnEditSource;
            text: i18nc("@label:button", "Edit SVG Source");
            onClicked: {
                manager.emitOpenTextEditor();
            }
            Layout.fillWidth: true;
            hoverEnabled: true;
            Kis.ToolTipBase {
                parent: btnEditSource;
                text: i18nc("@info:tooltip", "Edit the currently selected text as SVG XML.");
                visible: parent.hovered;
            }
        }
        Button {
            id: btnGlyphPalette;
            text: i18nc("@label:button", "Glyph Palette");
            onClicked: {
                manager.emitGlyphPalette();
            }
            Layout.fillWidth: true;
            hoverEnabled: true;
            Kis.ToolTipBase {
                parent: btnGlyphPalette;
                text: i18nc("@info:tooltip", "Open the Glyph Palette, which provides access to alternative OpenType or Unicode Variations of glyphs, as well as a Character Map.");
                visible: parent.hovered;
            }
            palette: themedControl.palette;
        }

        Kis.ToolSeparatorBase {
            orientation: Qt.Horizontal;
            Layout.fillWidth: true;
        }
        Label {
            text: i18nc("@label:group", "Convert Type...");
        }

        ButtonGroup {
            id: textType;

            exclusive: true;
            function updateTextTypes() {
                btnPreFormatted.checked = (manager.textType === btnPreFormatted.type);
                btnInlineWrapped.checked = (manager.textType === btnInlineWrapped.type);
                btnPrepositioned.checked = (manager.textType === btnPrepositioned.type);

                if (manager.textType < 0) {
                    btnPreFormatted.enabled = false;
                    btnInlineWrapped.enabled = false;
                    btnPrepositioned.enabled = false;
                } else {
                    btnPreFormatted.enabled = true;
                    btnInlineWrapped.enabled = true;
                    btnPrepositioned.enabled = true;
                }
            }
        }

        Button {
            id: btnPreFormatted;
            property int type: SvgTextToolOptionsManager.PreformattedText;
            text: i18nc("@label:button", "Preformatted");
            Layout.fillWidth: true;
            ButtonGroup.group: textType;
            onClicked: manager.textType = type;
            hoverEnabled: true;
            Kis.ToolTipBase {
                parent: btnPreFormatted;
                text: i18nc("@info:tooltip", "The text will not wrap and preserves spaces and new lines. A line break is inserted so the lines from wrapping or positioning remain separated.");
                visible: parent.hovered;
            }
            Kis.ThemedControl {
                id: convertButtonPalette;
            }
            palette: convertButtonPalette.palette;
        }
        Button {
            id: btnInlineWrapped;
            text: i18nc("@label:button", "Inline Wrapped");
            property int type: SvgTextToolOptionsManager.InlineWrap;
            Layout.fillWidth: true;
            ButtonGroup.group: textType;
            onClicked: manager.textType = type;
            hoverEnabled: true;
            Kis.ToolTipBase {
                parent: btnInlineWrapped;
                text: i18nc("@info:tooltip", "Text wraps inside a fixed length, spaces and new lines are preserved. New lines are not removed.");
                visible: parent.hovered;
            }
            palette: convertButtonPalette.palette;
        }
        Button {
            id: btnPrepositioned;
            text: i18nc("@label:button", "Prepositioned");
            property int type: SvgTextToolOptionsManager.PrePositionedText;
            Layout.fillWidth: true;
            ButtonGroup.group: textType;
            onClicked: manager.textType = type;
            hoverEnabled: true;
            Kis.ToolTipBase {
                parent: btnPrepositioned;
                text: i18nc("@info:tooltip", "Text uses SVG 1.1 character transforms to position each line. Spaces and new lines are collapsed. This is for backward compatibility with SVG 1.1.");
                visible: parent.hovered;
            }
            palette: convertButtonPalette.palette;
        }

        Kis.ToolSeparatorBase {
            orientation: Qt.Horizontal;
            Layout.fillWidth: true;
            visible: manager.showDebug;
        }
        Label {
            text: i18nc("@label:group", "Debug");
            visible: manager.showDebug;
        }

        CheckBox {
            id: chkDebugCharacter;
            text: i18nc("@option:check", "Show Char Bounding Boxes");
            checked: manager.showCharacterDebug;
            onCheckedChanged: {
                manager.showCharacterDebug = checked;
            }
            Layout.fillWidth: true;
            visible: manager.showDebug;
        }
        CheckBox {
            id: chkDebugLine;
            text: i18nc("@option:check", "Show Line Boxes");
            checked: manager.showLineDebug;
            onCheckedChanged: {
                manager.showLineDebug = checked;
            }
            Layout.fillWidth: true;
            visible: manager.showDebug;
        }

    }
}
