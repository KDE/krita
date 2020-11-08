# -*- coding: utf-8 -*-
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

"""
This module will be a collection of functions to hook into the GUI of Scribus.

Currently it only provides functions to add items to a menubar.
Support for the toolbar, statusbar and dockarea have still to be implemented.
I have to think about how to provide this stuff to QtQml.
"""

from PyQt5.QtWidgets import QApplication, QMenu

import mikro


class MenuHooks(object):

    """
    This class lets extension-scripts hook into the main menu of Scribus.
    """

    def __init__(self, window=None):
        self.window = window or Scripter.dialogs.mainWindow.qt
        self.menubar = self.window.menuBar()
        self.menus = []

    def createMenu(self, title):
        m = QMenu(title)
        self.menus.append(m)
        self.menubar.addMenu(m)
        return m

    def iter_menus(self):
        for action in self.menubar.actions():
            menu = action.menu()
            if menu:
                yield menu

    def iter_inner_menus(self, menu):
        for action in menu.actions():
            menu = action.menu()
            if menu:
                yield menu

    def findMenu(self, title):
        """
        find a menu with a given title

        @type  title: string
        @param title: English title of the menu
        @rtype:       QMenu
        @return:      None if no menu was found, else the menu with title
        """
        # See also http://pyqt.sourceforge.net/Docs/PyQt5/i18n.html#differences-between-pyqt5-and-qt
        title = QApplication.translate(mikro.classname(self.window), title)
        for menu in self.iter_menus():
            if menu.title() == title:
                return menu
            for innerMenu in self.iter_inner_menus(menu):
                if innerMenu.title() == title:
                    return innerMenu

    def actionForMenu(self, menu):
        for action in self.menubar.actions():
            if action.menu() == menu:
                return action

    def insertMenuBefore(self, before_menu, new_menu):
        """
        Insert a menu after another menu in the menubar

        @type: before_menu QMenu instance or title string of menu
        @param before_menu: menu which should be after the newly inserted menu
        @rtype: QAction instance
        @return: action for inserted menu
        """
        if isinstance(before_menu, basestring):
            before_menu = self.findMenu(before_menu)
        before_action = self.actionForMenu(before_menu)
        # I have no clue why QMenuBar::insertMenu only allows
        # to insert before another menu and not after a menu...
        new_action = self.menubar.insertMenu(before_action, new_menu)
        return new_action

    def menuAfter(self, menu):
        # This method is needed for insertMenuAfter because
        # QMenuBar.insertMenu can only insert before another menu
        previous = None
        for m in self.iter_menus():
            if previous and previous == menu:
                return m
            previous = m

    def appendMenu(self, menu):
        """
          Probably not that useful
          because it will add a menu after the help menu
        """
        action = self.menubar.addMenu(menu)
        return action

    def insertMenuAfter(self, after_menu, new_menu):
        """
        Insert a menu before another menu in the menubar
        """
        if isinstance(after_menu, basestring):
            after_menu = self.findMenu(after_menu)
        after_after_menu = self.menuAfter(after_menu)
        if after_after_menu:
            return self.insertMenuBefore(after_after_menu, new_menu)
        else:
            return self.appendMenu(new_menu)

    def appendItem(self, menu, item, *extra_args):
        if isinstance(menu, basestring):
            title = menu
            menu = self.findMenu(title)
            if not menu:
                raise ValueError("Menu %r not found" % title)
        if isinstance(item, QMenu):
            action = menu.addMenu(item)
        else:
            action = menu.addAction(item, *extra_args)
        return action

    def appendSeparator(self, menu):
        if isinstance(menu, basestring):
            menu = self.findMenu(menu)
        menu.addSeparator()
