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
    property alias letterSpacing: letterSpacingSpn.value;

    onPropertiesUpdated: {
        blockSignals = true;
        letterSpacing = properties.letterSpacing.value;
        blockSignals = false;
    }
    onLetterSpacingChanged: {
        if (!blockSignals) {
            properties.letterSpacing.value = letterSpacing;
        }
    }

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.letterSpacingState === KoSvgTextPropertiesModel.PropertySet;
        }

        Label {
            text: i18nc("@label:spinbox", "Letter Spacing:")
        }

        SpinBox {
            id: letterSpacingSpn
            Layout.fillWidth: true;
        }

        ComboBox {
            model: ["Pt", "Em", "Ex"]
        }
    }
}
