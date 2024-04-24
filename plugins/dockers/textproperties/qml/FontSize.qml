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

    property alias fontSize: fontSizeSpn.value;
    visible: properties.fontSizeState !== KoSvgTextPropertiesModel.PropertyUnset;

    onPropertiesUpdated: {
        blockSignals = true;
        fontSize = properties.fontSize.value;
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
            text: i18nc("@label:spinbox", "Font Size:")
        }

        SpinBox {
            id: fontSizeSpn
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
        }
    }
}
