/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

CollapsibleGroupProperty {
    propertyName: i18nc("@label", "Font Style");

    property alias fontWeight: fontWeightSpn.value;
    property alias fontWidth: fontStretchSpn.value;
    property int fontSlant: KoSvgTextPropertiesModel.StyleNormal;

    onFontSlantChanged: {
        fontSlantCmb.currentIndex = fontSlantCmb.indexOfValue(fontSlant);
        if (!blockSignals) {
            properties.fontStyle = fontSlant;
        }
    }
    property alias fontOptical: opticalSizeCbx.checked;

    onPropertiesUpdated: {
        blockSignals = true;
        fontWeight = properties.fontWeight;
        fontWidth = properties.fontWidth;
        fontOptical = properties.fontOpticalSizeLink;
        fontSlant = properties.fontStyle;
        visible = properties.fontWeightState !== KoSvgTextPropertiesModel.PropertyUnset
                 || properties.fontStyleState !== KoSvgTextPropertiesModel.PropertyUnset
                 || properties.fontWidthState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }
    onFontWeightChanged: {
        if (!blockSignals) {
            properties.fontWeight = fontWeight;
        }
    }

    onFontWidthChanged: {
        if (!blockSignals) {
            properties.fontWidth = fontWidth;
        }
    }

    onFontOpticalChanged: {
        if (!blockSignals) {
            properties.fontOpticalSizeLink = fontOptical;
        }
    }

    titleItem: ComboBox {
        id: styleCmb;
        Layout.fillWidth: true;
    }

    contentItem: GridLayout {
        columns: 3
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: 5;

        RevertPropertyButton {
            revertEnabled: properties.fontWeightState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.fontWeightState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: i18nc("@label:spinbox", "Weight:")
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            horizontalAlignment: Text.AlignRight;
            Layout.preferredWidth: implicitWidth;
            elide: Text.ElideRight;
        }

        SpinBox {
            id: fontWeightSpn
            from: 0;
            to: 1000;
            editable: true;
            Layout.fillWidth: true
        }


        RevertPropertyButton {
            revertEnabled: properties.fontWidthState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.fontWidthState = KoSvgTextPropertiesModel.PropertyUnset;
        }
        Label {
            text: i18nc("@label:spinbox", "Width:")
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            horizontalAlignment: Text.AlignRight;
            Layout.preferredWidth: implicitWidth;
            elide: Text.ElideRight;
        }

        SpinBox {
            id: fontStretchSpn
            from: 0;
            to: 200;
            editable: true;
            Layout.fillWidth: true
        }

        RevertPropertyButton {
            revertEnabled: properties.fontStyleState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.fontStyleState = KoSvgTextPropertiesModel.PropertyUnset;
        }
        Label {
            text: i18nc("@label:listbox", "Slant:")
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            horizontalAlignment: Text.AlignRight;
            Layout.preferredWidth: implicitWidth;
            elide: Text.ElideRight;
        }

        ComboBox {
            id: fontSlantCmb
            model: [
                {text: i18nc("@label:inlistbox", "Normal"), value: KoSvgTextPropertiesModel.StyleNormal},
                {text: i18nc("@label:inlistbox", "Italic"), value: KoSvgTextPropertiesModel.StyleItalic},
                {text: i18nc("@label:inlistbox", "Oblique"), value: KoSvgTextPropertiesModel.StyleOblique}
            ]
            Layout.fillWidth: true
            textRole: "text";
            valueRole: "value";
            onActivated: fontSlant = currentValue;
        }

        RevertPropertyButton {
            revertEnabled: properties.fontOpticalSizeLinkState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.fontOpticalSizeLinkState = KoSvgTextPropertiesModel.PropertyUnset;
        }
        Item {
        width: 1;
        height: 1;}
        CheckBox {
            id: opticalSizeCbx
            text: i18nc("@option:check", "Optical Size")
            Layout.fillWidth: true
        }

    }
}
