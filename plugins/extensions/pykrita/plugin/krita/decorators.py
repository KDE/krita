# -*- coding: utf-8 -*-
# SPDX-FileCopyrightText: 2006 Paul Giannaros <paul@giannaros.org>
# SPDX-FileCopyrightText: 2013 Shaheed Haque <srhaque@theiet.org>
# SPDX-FileCopyrightText: 2013 Alex Turbov <i.zaufi@gmail.com>
# SPDX-FileCopyrightText: 2014-2016 Boudewijn Rempt <boud@valdyas.org>
#
# SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only

'''Decorators used in plugins'''

import functools
import inspect
import sys
import traceback

from PyQt5 import QtCore, QtGui, QtWidgets

import pykrita

from .api import *

#
# initialization related stuff
#


def pykritaEventHandler(event):
    def _decorator(func):
        setattr(pykrita, event, func)
        del func
    return _decorator


def _callAll(plugin, functions, *args, **kwargs):
    if plugin in functions:
        for f in functions[plugin]:
            try:
                f(*args, **kwargs)
            except:
                traceback.print_exc()
                sys.stderr.write('\n')
                # TODO Return smth to a caller, so in case of
                # failed initialization it may report smth to the
                # C++ level and latter can show an error to the user...
                continue


def _simpleEventListener(func):
    # automates the most common decorator pattern: calling a bunch
    # of functions when an event has occurred
    func.functions = dict()
    func.fire = functools.partial(_callAll, functions=func.functions)
    func.clear = func.functions.clear
    return func


def _registerCallback(plugin, event, func):
    if plugin not in event.functions:
        event.functions[plugin] = set()

    event.functions[plugin].add(func)
    return func


@_simpleEventListener
def init(func):
    ''' The function will be called when particular plugin has loaded
        and the configuration has been initiated
    '''
    plugin = sys._getframe(1).f_globals['__name__']
    qDebug('@init: {}/{}'.format(plugin, func.__name__))
    return _registerCallback(plugin, init, func)


@_simpleEventListener
def unload(func):
    ''' The function will be called when particular plugin is being
        unloaded from memory. Clean up any widgets that you have added
        to the interface (toolviews etc).

        ATTENTION Be really careful trying to access any window, view
            or document from the @unload handler: in case of application
            quit everything is dead already!
    '''
    plugin = sys._getframe(1).f_globals['__name__']
    qDebug('@unload: {}/{}'.format(plugin, func.__name__))

    def _module_cleaner():
        qDebug('@unload/cleaner: {}/{}'.format(plugin, func.__name__))
        if plugin in init.functions:
            qDebug('@unload/init-cleaner: {}/{}'.format(plugin, func.__name__))
            del init.functions[plugin]

        func()

    return _registerCallback(plugin, unload, _module_cleaner)
