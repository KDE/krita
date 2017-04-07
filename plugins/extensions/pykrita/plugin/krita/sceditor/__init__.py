# -*- coding: utf-8 -*-


editor_main_window = None


def launch(parent=None):
    global editor_main_window
    if not editor_main_window:
        from sceditor.mainwindow import EditorMainWindow
        editor_main_window = EditorMainWindow(parent)
        editor_main_window.resize(640,480)
    editor_main_window.show()

