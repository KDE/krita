/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import org.krita.components 1.0 as Kis

Kis.ResourcePopup {
    id: resourceCmb;

    resourceType: "fontfamilies";
    addResourceRowVisible: false;
    resourceDelegate: Kis.FontFamilyDelegate {
        id: fontDelegate;
        resourceView: resourceCmb.view;
        locales: resourceCmb.locales;
        onResourceLeftClicked: {
            resourceCmb.activated();
        }
    }
    view.preferredHeight: 300; // roughly 3.5 times the font delegate size;
}
