"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtGui import QColor, QTextCharFormat, QFont


def _format(color, style='', darker=100, lighter=100):
    """Return a QTextCharFormat with the given attributes.
    """
    _color = QColor(color)
    _color = _color.darker(darker)
    _color = _color.lighter(lighter)

    fmt = QTextCharFormat()
    fmt.setForeground(_color)
    if 'bold' in style:
        fmt.setFontWeight(QFont.Bold)
    if 'italic' in style:
        fmt.setFontItalic(True)

    return fmt


class DefaultSyntaxStyle(object):

    # Syntax styles that combines with dark backgrounds
    STYLES = {
        'keyword': _format('cyan'),
        'operator': _format('orange'),
        'brace': _format('gray'),
        'defclass': _format('black', 'bold'),
        'string': _format('magenta'),
        'string2': _format('darkMagenta'),
        'comment': _format('darkGreen', 'italic'),
        'self': _format('black', 'italic'),
        'numbers': _format('brown'),
        'background': _format('white'),
        'foreground': _format('black'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]


class PythonVimSyntaxStyle(object):

    """ It based in the colorschemme of the Vim editor for python code http://www.vim.org/scripts/script.php?script_id=790 """
    # Syntax styles that combines with dark backgrounds
    STYLES = {
        'keyword': _format('yellow', darker=125),
        'operator': _format('magenta', darker=150),
        'brace': _format('white'),
        'defclass': _format('orange', 'bold'),
        'string': _format('green', lighter=160),
        'string2': _format('lightGray', 'italic', darker=120),
        'comment': _format('gray', 'italic'),
        'self': _format('blue', lighter=170),
        'numbers': _format('yellow', lighter=130),
        'background': _format('black'),
        'foreground': _format('white'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]


class BreezeDarkSyntaxStyle(object):

    """ Based on KDE Breeze widget style """
    # A dark syntax style.
    STYLES = {
        'keyword': _format('#eff0f1', 'bold'),
        'operator': _format('#eff0f1'),
        'brace': _format('#eff0f1'),
        'defclass': _format('#27ae60', 'bold'),
        'string': _format('#da4453'),
        'string2': _format('#da4453'),
        'comment': _format('#7f8c8d', 'italic'),
        'self': _format('#3daee9'),
        'numbers': _format('#f67400'),
        'background': _format('#232629'),
        'foreground': _format('#eff0f1'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]


class BreezeLightSyntaxStyle(object):

    """ Based on KDE Breeze widget style """
    # A light syntax style.
    STYLES = {
        'keyword': _format('#31363b', 'bold'),
        'operator': _format('#31363b'),
        'brace': _format('#31363b'),
        'defclass': _format('#27ae60', 'bold'),
        'string': _format('#da4453'),
        'string2': _format('#da4453'),
        'comment': _format('#7f8c8d', 'italic'),
        'self': _format('#3daee9'),
        'numbers': _format('#f67400'),
        'background': _format('#fcfcfc'),
        'foreground': _format('#31363b'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]


class BlenderSyntaxStyle(object):

    """ Based on KDE Breeze widget style """
    # A light syntax style.
    STYLES = {
        'keyword': _format('#606002'),
        'operator': _format('#4c4c4c'),
        'brace': _format('#4c4c4c'),
        'defclass': _format('#000000'),
        'string': _format('#650202'),
        'string2': _format('#650202'),
        'comment': _format('#006432'),
        'self': _format('#000000'),
        'numbers': _format('#0000c8'),
        'background': _format('#999999'),
        'foreground': _format('#000000'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]


class SolarizedDarkSyntaxStyle(object):

    """ Based on http://ethanschoonover.com/solarized """
    # A dark syntax style.
    STYLES = {
        'keyword': _format('#6b9500'),
        'operator': _format('#839496'),
        'brace': _format('#839496'),
        'defclass': _format('#248bd2', 'bold'),
        'string': _format('#29a198'),
        'string2': _format('#29a198'),
        'comment': _format('#586e75', 'italic'),
        'self': _format('#248bd2'),
        'numbers': _format('#b58900'),
        'background': _format('#002a35'),
        'foreground': _format('#839496'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]


class SolarizedLightSyntaxStyle(object):

    """ Based on http://ethanschoonover.com/solarized """
    # A light syntax style.
    STYLES = {
        'keyword': _format('#6b9500'),
        'operator': _format('#839496'),
        'brace': _format('#839496'),
        'defclass': _format('#248bd2', 'bold'),
        'string': _format('#29a198'),
        'string2': _format('#29a198'),
        'comment': _format('#586e75', 'italic'),
        'self': _format('#248bd2'),
        'numbers': _format('#b58900'),
        'background': _format('#fdf6e3'),
        'foreground': _format('#839496'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]
