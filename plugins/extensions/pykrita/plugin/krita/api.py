# -*- coding: utf-8 -*-
# SPDX-FileCopyrightText: 2006 Paul Giannaros <paul@giannaros.org>
# SPDX-FileCopyrightText: 2013 Shaheed Haque <srhaque@theiet.org>
# SPDX-FileCopyrightText: 2013 Alex Turbov <i.zaufi@gmail.com>
# SPDX-FileCopyrightText: 2014-2016 Boudewijn Rempt <boud@valdyas.org>
#
# SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only

'''Provide shortcuts to access krita internals from plugins'''

import contextlib
import os
import sys

from PyKrita.krita import *
import pykrita

def qDebug(text):
    '''Use KDE way to show debug info

        TODO Add a way to control debug output from partucular plugins (?)
    '''
    plugin = sys._getframe(1).f_globals['__name__']
    pykrita.qDebug('{}: {}'.format(plugin, text))
