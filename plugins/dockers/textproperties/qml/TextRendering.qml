/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.flake.text 1.0

TextPropertyBase {
    propertyTitle: i18nc("@title:group", "Text Rendering");
    propertyName: "text-rendering";
    propertyType: TextPropertyConfigModel.Paragraph;
    toolTip: i18nc("@info:tooltip",
                   "Text Rendering controls the hinting and rendering style for the property");
    searchTerms: i18nc("comma separated search terms for the text-rendering property, matching is case-insensitive",
                       "text-rendering, hinting, anti-aliasing");
    property int textRendering;

    Connections {
        target: properties;
        function onTextRenderingChanged() {
            updateRendering();
            updateVisibility();
        }

        function onTextRenderingStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateRendering();
        updateVisibility();
    }

    function updateVisibility() {
        propertyState = [properties.textRenderingState];
        setVisibleFromProperty();
    }

    function updateRendering() {
        blockSignals = true;
        textRendering = properties.textRendering;
        blockSignals = false;
    }

    onTextRenderingChanged: {
        renderingCmb.currentIndex = renderingCmb.indexOfValue(textRendering);
        if (!blockSignals) {
            properties.textRendering = textRendering;
        }
    }

    onEnableProperty: properties.textRenderingState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.textRenderingState;
            onClicked: properties.textRenderingState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.textRenderingState === KoSvgTextPropertiesModel.PropertyTriState;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        SqueezedComboBox {
            model: [
                {text: i18nc("@label:inlistbox", "Auto"), value: KoSvgText.RenderingAuto, icon: "",
                    toolTip: i18nc("@info:tooltip", "Same as geometric precision, uses no hinting and avoid loading internal bitmaps.")},
                {text: i18nc("@label:inlistbox", "Optimize Speed"), value: KoSvgText.RenderingOptimizeSpeed, icon: "qrc:///light_select-pixel.svg",
                    toolTip: i18nc("@info:tooltip", "Renders without antialiasing.")},
                {text: i18nc("@label:inlistbox", "Optimize Legibility"), value: KoSvgText.RenderingOptimizeLegibility, icon: "",
                    toolTip: i18nc("@info:tooltip", "Uses 'light' hinting in horizontal and 'full' hinting in vertical.")},
                {text: i18nc("@label:inlistbox", "Geometric Precision"), value: KoSvgText.RenderingGeometricPrecision, icon: "qrc:///light_select-shape.svg",
                    toolTip: i18nc("@info:tooltip", "No hinting and avoids loading bitmap strikes unless the font has no paths.")},

            ]
            Layout.fillWidth: true;
            id: renderingCmb;
            textRole: "text";
            valueRole: "value";
            iconRole: "icon";
            iconSize: 16;
            toolTipRole: "toolTip";
            onActivated: textRendering = currentValue;
            wheelEnabled: true;
        }
    }
}
