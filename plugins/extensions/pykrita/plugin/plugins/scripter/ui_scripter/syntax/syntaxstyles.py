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
    }

    def __getitem__(self, key):
        return self.STYLES[key]
