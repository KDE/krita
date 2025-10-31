/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

/*
  \qmltype PopupBase
  Workaround to allow us to use a PopupWidget when we're inside a QQuickWidget,
  and otherwise a regular QtQuickControls Popup.
  */
Popup {
}
