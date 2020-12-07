# -*- coding: utf-8 -*-
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

editor_main_window = None


def launch(parent=None):
    global editor_main_window
    if not editor_main_window:
        from sceditor.mainwindow import EditorMainWindow
        editor_main_window = EditorMainWindow(parent)
        editor_main_window.resize(640, 480)
    editor_main_window.show()
