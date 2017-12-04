from PyQt5.QtGui import QColor, QTextCharFormat, QFont


def format(color, style='', darker=100, lighter=100):
    """Return a QTextCharFormat with the given attributes.
    """
    _color = QColor(color)
    _color = _color.darker(darker)
    _color = _color.lighter(lighter)

    _format = QTextCharFormat()
    _format.setForeground(_color)
    if 'bold' in style:
        _format.setFontWeight(QFont.Bold)
    if 'italic' in style:
        _format.setFontItalic(True)

    return _format


class DefaultSyntaxStyle(object):

    # Syntax styles that combines with dark backgrounds
    STYLES = {
        'keyword': format('cyan'),
        'operator': format('orange'),
        'brace': format('gray'),
        'defclass': format('black', 'bold'),
        'string': format('magenta'),
        'string2': format('darkMagenta'),
        'comment': format('darkGreen', 'italic'),
        'self': format('black', 'italic'),
        'numbers': format('brown'),
        'background' : format('white'),
        'foreground' : format('black'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]


class PythonVimSyntaxStyle(object):

    """ It based in the colorschemme of the Vim editor for python code http://www.vim.org/scripts/script.php?script_id=790 """
    # Syntax styles that combines with dark backgrounds
    STYLES = {
        'keyword': format('yellow', darker=125),
        'operator': format('magenta', darker=150),
        'brace': format('white'),
        'defclass': format('orange', 'bold'),
        'string': format('green', lighter=160),
        'string2': format('lightGray', 'italic', darker=120),
        'comment': format('gray', 'italic'),
        'self': format('blue', lighter=170),
        'numbers': format('yellow', lighter=130),
        'background' : format('black'),
        'foreground' : format('white'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]

class BreezeDarkSyntaxStyle(object):

    """ Based on KDE Breeze widget style """
    # A dark syntax style.
    STYLES = {
        'keyword': format('#eff0f1', 'bold'),
        'operator': format('#eff0f1'),
        'brace': format('#eff0f1'),
        'defclass': format('#27ae60', 'bold'),
        'string': format('#da4453'),
        'string2': format('#da4453'),
        'comment': format('#7f8c8d', 'italic'),
        'self': format('#3daee9'),
        'numbers': format('#f67400'),
        'background' : format('#232629'),
        'foreground' : format('#eff0f1'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]

class BreezeLightSyntaxStyle(object):

    """ Based on KDE Breeze widget style """
    # A light syntax style.
    STYLES = {
        'keyword': format('#31363b', 'bold'),
        'operator': format('#31363b'),
        'brace': format('#31363b'),
        'defclass': format('#27ae60', 'bold'),
        'string': format('#da4453'),
        'string2': format('#da4453'),
        'comment': format('#7f8c8d', 'italic'),
        'self': format('#3daee9'),
        'numbers': format('#f67400'),
        'background' : format('#fcfcfc'),
        'foreground' : format('#31363b'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]

class BlenderSyntaxStyle(object):

    """ Based on KDE Breeze widget style """
    # A light syntax style.
    STYLES = {
        'keyword': format('#606002'),
        'operator': format('#4c4c4c'),
        'brace': format('#4c4c4c'),
        'defclass': format('#000000'),
        'string': format('#650202'),
        'string2': format('#650202'),
        'comment': format('#006432'),
        'self': format('#000000'),
        'numbers': format('#0000c8'),
        'background' : format('#999999'),
        'foreground' : format('#000000'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]

class SolarizedDarkSyntaxStyle(object):

    """ Based on http://ethanschoonover.com/solarized """
    # A dark syntax style.
    STYLES = {
        'keyword': format('#6b9500'),
        'operator': format('#839496'),
        'brace': format('#839496'),
        'defclass': format('#248bd2', 'bold'),
        'string': format('#29a198'),
        'string2': format('#29a198'),
        'comment': format('#586e75', 'italic'),
        'self': format('#248bd2'),
        'numbers': format('#b58900'),
        'background' : format('#002a35'),
        'foreground' : format('#839496'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]

class SolarizedLightSyntaxStyle(object):

    """ Based on http://ethanschoonover.com/solarized """
    # A light syntax style.
    STYLES = {
        'keyword': format('#6b9500'),
        'operator': format('#839496'),
        'brace': format('#839496'),
        'defclass': format('#248bd2', 'bold'),
        'string': format('#29a198'),
        'string2': format('#29a198'),
        'comment': format('#586e75', 'italic'),
        'self': format('#248bd2'),
        'numbers': format('#b58900'),
        'background' : format('#fdf6e3'),
        'foreground' : format('#839496'),
    }

    def __getitem__(self, key):
        return self.STYLES[key]
