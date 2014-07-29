# -*- coding: utf-8 -*-
# Copyright (C) 2006 Paul Giannaros <paul@giannaros.org>
# Copyright (C) 2013 Shaheed Haque <srhaque@theiet.org>
# Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) version 3.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; see the file COPYING.LIB.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

'''Configuration related stuff'''

import pykrita                                                 # Built-in module
import sys


class Configuration:
    '''A Configuration object provides a plugin-specific persistent
    configuration dictionary. The configuration is saved and loaded from disk
    automatically.

    Do not instantiate your own Configuration object; a plugin simply uses
    pykrita.configuration and the class automatically creates a plugin-specific
    dictionary to support it.

    Use a string key. Any Python type that can be pickled is can be used as a
    value -- dictionaries, lists, numbers, strings, sets, and so on.
    '''

    def __init__(self, root):
        self.root = root

    def __getitem__(self, key):
        plugin = sys._getframe(1).f_globals['__name__']
        return self.root.get(plugin, {})[key]

    def __setitem__(self, key, value):
        plugin = sys._getframe(1).f_globals['__name__']
        if plugin not in self.root:
            self.root[plugin] = {}
        self.root[plugin][key] = value

    def __delitem__(self, key):
        plugin = sys._getframe(1).f_globals['__name__']
        del self.root.get(plugin, {})[key]

    def __contains__(self, key):
        plugin = sys._getframe(1).f_globals['__name__']
        return key in self.root.get(plugin, {})

    def __len__(self):
        plugin = sys._getframe(1).f_globals['__name__']
        return len(self.root.get(plugin, {}))

    def __iter__(self):
        plugin = sys._getframe(1).f_globals['__name__']
        return iter(self.root.get(plugin, {}))

    def __str__(self):
        plugin = sys._getframe(1).f_globals['__name__']
        return str(self.root.get(plugin, {}))

    def __repr__(self):
        plugin = sys._getframe(1).f_globals['__name__']
        return repr(self.root.get(plugin, {}))

    def keys(self):
        '''Return the keys from the configuration dictionary.'''
        plugin = sys._getframe(1).f_globals['__name__']
        return self.root.get(plugin, {}).keys()

    def values(self):
        '''Return the values from the configuration dictionary.'''
        plugin = sys._getframe(1).f_globals['__name__']
        return self.root.get(plugin, {}).values()

    def items(self):
        '''Return the items from the configuration dictionary.'''
        plugin = sys._getframe(1).f_globals['__name__']
        return self.root.get(plugin, {}).items()

    def get(self, key, default=None):
        '''Fetch a configuration item using it's string key, returning
        a given default if not found.

        Parameters:
            * key -             String key for item.
            * default -         Value to return if key is not found.

        Returns:
            The item value for key, or the given default if not found.
        '''
        plugin = sys._getframe(1).f_globals['__name__']
        try:
            return self.root.get(plugin, {})[key]
        except KeyError:
            return default

    def pop(self, key):
        '''Delete a configuration item using it's string key.

        Parameters:
            * key -             String key for item.
        Returns:
            The value of the removed item.
        Throws:
            KeyError if key doesn't exist.
        '''
        plugin = sys._getframe(1).f_globals['__name__']
        value = self.root.get(plugin, {})[key]
        del self.root.get(plugin, {})[key]
        return value

    def save(self):
        pykrita.saveConfiguration()

    def _name(self):
        return sys._getframe(1).f_globals['__name__']


globalConfiguration = pykrita.configuration
'''Configuration for all plugins.

This can also be used by one plugin to access another plugin's configurations.
'''

configuration = Configuration(pykrita.configuration)
'''Persistent configuration dictionary for this plugin.'''


sessionConfiguration = Configuration(pykrita.sessionConfiguration)
'''Per session persistent configuration dictionary for this plugin.'''
