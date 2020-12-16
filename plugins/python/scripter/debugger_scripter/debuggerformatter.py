"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
import re
import inspect


def format_data(data):
    globals()['types'] = __import__('types')

    exclude_keys = ['copyright', 'credits', 'False',
                    'True', 'None', 'Ellipsis', 'quit',
                    'QtCriticalMsg', 'krita_path',
                    'QtWarningMsg', 'QWIDGETSIZE_MAX',
                    'QtFatalMsg', 'PYQT_CONFIGURATION',
                    'on_load', 'PYQT_VERSION', 'on_pykrita_unloading',
                    'on_unload', 'QT_VERSION', 'QtInfoMsg',
                    'PYQT_VERSION_STR', 'qApp', 'QtSystemMsg',
                    'QtDebugMsg', 'on_pykrita_loaded', 'QT_VERSION_STR']
    exclude_valuetypes = [types.BuiltinFunctionType,
                          types.BuiltinMethodType,
                          types.ModuleType,
                          types.FunctionType]

    return [{k: {'value': str(v), 'type': str(type(v))}} for k, v in data.items() if not (k in exclude_keys or
                                                                                          type(v) in exclude_valuetypes or
                                                                                          re.search(r'^(__).*\1$', k) or
                                                                                          inspect.isclass(v) or
                                                                                          inspect.isfunction(v))]
