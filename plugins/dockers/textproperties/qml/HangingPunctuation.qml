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
    property alias hangStart: paragraphStartCbx.checked;
    property alias hangEnd: paragraphEndCbx.checked;
    property var hangComma;
    onHangCommaChanged: lineEndCmb.currentIndex = lineEndCmb.indexOfValue(hangComma)
    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        Item {
            width: firstColumnWidth;
            height: firstColumnWidth;
            ToolButton {
                id: revert;
                icon.width: 22;
                icon.height: 22;
                display: AbstractButton.IconOnly
                icon.source: "qrc:///light_view-refresh.svg"
            }
        }

        Label {
            text: i18nc("@label", "Hanging-punctuation:")
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Hang First")
            id: paragraphStartCbx;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: i18nc("@label:listbox", "Line End:")
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        ComboBox {
            model: [
                {text: i18nc("@label:inlistbox", "No Hanging"), value: KoSvgTextPropertiesModel.NoHang},
                {text: i18nc("@label:inlistbox", "Allow hanging commas"), value: KoSvgTextPropertiesModel.AllowHang},
                {text: i18nc("@label:inlistbox", "Force hanging commas"), value: KoSvgTextPropertiesModel.ForceHang}
            ]
            Layout.fillWidth: true;
            id: lineEndCmb;
            textRole: "text";
            valueRole: "value";
            onActivated: hangComma = currentValue;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }

        CheckBox {
            text: i18nc("@option:check", "Hang Last")
            id: paragraphEndCbx;
        }

    }
}
