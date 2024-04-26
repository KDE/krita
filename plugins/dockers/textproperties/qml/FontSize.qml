/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

TextPropertyBase {
    propertyName: i18nc("@label:spinbox", "Font Size");

    property alias fontSize: fontSizeSpn.value;

    onPropertiesUpdated: {
        blockSignals = true;
        fontSize = properties.fontSize.value;
        visible = properties.fontSizeState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }
    onFontSizeChanged: {
        if (!blockSignals) {
            properties.fontSize.value = fontSize;
        }
    }

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.fontSizeState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.fontSizeState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
        }

        SpinBox {
            id: fontSizeSpn;
            editable: true;
            Layout.fillWidth: true;
        }

        ComboBox {
            model: [
                {text: i18nc("@label:inlistbox", "Pt"), value: 0},
                {text: i18nc("@label:inlistbox", "Em"), value: 1},
                {text: i18nc("@label:inlistbox", "Ex"), value: 2},
                {text: i18nc("@label:inlistbox", "%"), value: 3}
            ]
            textRole: "text";
            valueRole: "value";
            Layout.fillWidth: true;
        }
    }
}
