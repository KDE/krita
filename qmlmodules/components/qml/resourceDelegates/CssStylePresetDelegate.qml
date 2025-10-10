/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.components 1.0 as Kis

/**
  Item Delegate to handle CssStylePresets;
  */
ResourceDelegateBase {
    id: presetDelegate;

    property var meta: model.metadata;
    width: ListView.view.width;
    preferredHeight: minimumHeight * 3;
    minimumHeight: Math.max(missingFamily.implicitHeight, nameLabel.implicitHeight);

    Kis.FontFunctions {
        id: fontFunctions;
    }

    contentItem: GridLayout {
        id: mainGrid;
        implicitHeight: Math.max(presetDelegate.minimumHeight, presetDelegate.preferredHeight);
        columns: implicitHeight < (presetDelegate.minimumHeight*2)? 2: 1;
        Row {
            id: iconRow;
            Layout.fillWidth: true;
            Layout.maximumWidth: presetDelegate.width/mainGrid.columns;
            Layout.minimumHeight: implicitHeight;
            Layout.preferredHeight: implicitHeight;
            ToolButton {
                id: missingFamily;
                visible: typeof presetDelegate.meta.primary_font_family !== 'undefined'?
                             presetDelegate.meta.primary_font_family !== ""
                             && typeof fontFunctions.wwsFontFamilyNameVariant(presetDelegate.meta.primary_font_family) === 'undefined': false;
                icon.source: palette.window.hslLightness < 0.5? "qrc:///16_light_warning.svg": "qrc:///16_dark_warning.svg";
                icon.height: 8;
                icon.width: 8;
            }
            Label {
                id: nameLabel;
                palette: presetDelegate.palette;
                text: presetDelegate.model.name;
                elide: Text.ElideRight;
                width: (iconRow.width - (missingFamily.implicitWidth+iconRow.spacing));
                color: presetDelegate.highlighted? palette.highlightedText: palette.text;
            }
        }
        Kis.KoShapeQtQuickLabel {
            id: styleSample;
            Layout.fillHeight: true;
            Layout.fillWidth: true;
            Layout.minimumHeight: presetDelegate.minimumHeight;
            Layout.maximumWidth: (mainGrid.columns == 1)? presetDelegate.width: Math.min(presetDelegate.width/mainGrid.columns, styleSample.height*(minimumRect.width/minimumRect.height));
            alignment: presetDelegate.meta.sample_align;
            scalingType: presetDelegate.meta.style_type === "paragraph"?
                             Kis.KoShapeQtQuickLabel.FitWidth: Kis.KoShapeQtQuickLabel.Fit;
            svgData: presetDelegate.meta.sample_svg;
            foregroundColor: presetDelegate.highlighted? palette.highlightedText: palette.text;
        }
    }

    background: Rectangle {
        color: presetDelegate.highlighted? presetDelegate.palette.highlight: "transparent";
        border.color: presetDelegate.selected? presetDelegate.palette.highlight: presetDelegate.palette.mid;
        border.width: presetDelegate.selected? 2: 1;
    }

    MouseArea {
        id: delegateMouseArea;
        acceptedButtons: Qt.RightButton | Qt.LeftButton;
        anchors.fill: parent;
        hoverEnabled: true;
        preventStealing: true;
        onClicked: {
            if (mouse.button === Qt.RightButton) {
                resourceView.openContextMenu(mouse.x, mouse.y, parent.model.name, parent.model.index);
            } else {
                resourceView.applyHighlightedIndex();
                presetDelegate.resourceLeftClicked();
            }
        }
        onDoubleClicked: {
            resourceView.applyHighlightedIndex();
            presetDelegate.resourceDoubleClicked();
        }
        onContainsMouseChanged: {
            if (containsMouse) {
                resourceView.highlightedIndex = parent.model.index;
            }
        }

        readonly property string missingFontName : i18nc("%1 is name of a font family", "Font Family \"%1\" is missing on this machine. This style preset may not work correctly.", presetDelegate.meta.primary_font_family);
        Kis.ToolTipBase {
            text: (typeof presetDelegate.meta.description !== 'undefined'? presetDelegate.model.name + "\n" + presetDelegate.meta.description: presetDelegate.model.name) + (missingFamily.visible?  "\n"+ delegateMouseArea.missingFontName: "");
            visible: delegateMouseArea.containsMouse;
        }

    }
}
