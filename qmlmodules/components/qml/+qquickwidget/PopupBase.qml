/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import org.krita.components 1.0 as Kis

/*
  \qmltype PopupBase
  Workaround to allow us to use a PopupWidget when we're inside a QQuickWidget,
  and otherwise a regular QtQuickControls Popup.

  Note: you'll have to manually implement/forward popup properties as necessary.
  Unfortunately we cannot assign an id to rootControl at runtime, so we cannot
  use Connections or alias properties.
  */
Kis.PopupWidget {
    id: control;

    property double width;
    property double height;
    property double padding;
    property Item contentItem;
    property var palette;

    property double contentWidth;
    property double contentHeight;
    property double implicitWidth;
    property double implicitHeight;

    property Item dummy: Item {
        id: dummyItem;

        Connections {
            target: control;
            function onWidthChanged() {
                if (control.rootControl) {
                    control.rootControl.width = control.width;
                }
            }

            function onHeightChanged() {
                if (control.rootControl) {
                    control.rootControl.height = control.height;
                }
            }
        }

        Binding {
            target: control;
            property: "contentWidth";
            value: control.contentItem.width;
        }
        Binding {
            target: control;
            property: "contentHeight";
            value: control.contentItem.height;
        }
        Binding {
            target: control;
            property: "implicitWidth";
            when: typeof control.rootControl != 'undefined';
            value: control.rootControl.implicitWidth;
        }
        Binding {
            target: control;
            property: "implicitHeight";
            when: typeof control.rootControl != 'undefined';
            value: control.rootControl.implicitHeight;
        }

        Binding {
            target: control.rootControl;
            when: typeof control.rootControl != 'undefined';
            property: "padding";
            value: control.padding;
        }
        Binding {
            target: control.rootControl;
            when: typeof control.rootControl != 'undefined';
            property: "contentItem";
            value: control.contentItem;
        }
        Binding {
            target: control.rootControl;
            when: typeof control.rootControl != 'undefined';
            property: "palette";
            value: control.palette;
        }
    }

    property var closePolicy; // Dummy to avoid warnings.

}
