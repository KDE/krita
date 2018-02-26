"""
Copyright (c) 2017 Eliakin Costa <eliakim170@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
