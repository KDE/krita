#!/usr/bin/env python

"""
highlightedtextedit.py

A PyQt custom widget example for Qt Designer.

SPDX-FileCopyrightText: 2006 David Boddie <david@boddie.org.uk>
SPDX-FileCopyrightText: 2005-2006 Trolltech ASA. All rights reserved.

SPDX-License-Identifier: GPL-2.0-or-later
"""

from PyQt5 import QtCore, QtGui


class PythonHighlighter(QtGui.QSyntaxHighlighter):

    keywords = (
        "and",       "del",       "for",       "is",        "raise",
        "assert",    "elif",      "from",      "lambda",    "return",
        "break",     "else",      "global",    "not",       "try",
        "class",     "except",    "if",        "or",        "while",
        "continue",  "exec",      "import",    "pass",      "yield",
        "def",       "finally",   "in",        "print"
    )

    def __init__(self, edit):
        document = edit.document()
        QtGui.QSyntaxHighlighter.__init__(self, document)

        base_format = QtGui.QTextCharFormat()
        base_format.setFont(edit.font())

        self.base_format = base_format
        self.document = document

        self.updateHighlighter(base_format.font())

    def highlightBlock(self, text):

        self.setCurrentBlockState(0)

        if text.trimmed().isEmpty():
            self.setFormat(0, len(text), self.empty_format)
            return

        self.setFormat(0, len(text), self.base_format)

        startIndex = 0
        if self.previousBlockState() != 1:
            startIndex = self.multiLineStringBegin.indexIn(text)

        if startIndex > -1:
            self.highlightRules(text, 0, startIndex)
        else:
            self.highlightRules(text, 0, len(text))

        while startIndex >= 0:

            endIndex = self.multiLineStringEnd.indexIn(text,
                                                       startIndex + len(self.multiLineStringBegin.pattern()))
            if endIndex == -1:
                self.setCurrentBlockState(1)
                commentLength = text.length() - startIndex
            else:
                commentLength = endIndex - startIndex + \
                    self.multiLineStringEnd.matchedLength()
                self.highlightRules(text, endIndex, len(text))

            self.setFormat(startIndex, commentLength, self.multiLineStringFormat)
            startIndex = self.multiLineStringBegin.indexIn(text,
                                                           startIndex + commentLength)

    def highlightRules(self, text, start, finish):

        for expression, format in self.rules:

            index = expression.indexIn(text, start)
            while index >= start and index < finish:
                length = expression.matchedLength()
                self.setFormat(index, min(length, finish - index), format)
                index = expression.indexIn(text, index + length)

    def updateFonts(self, font):

        self.base_format.setFont(font)
        self.empty_format = QtGui.QTextCharFormat(self.base_format)
        # self.empty_format.setFontPointSize(font.pointSize()/4.0)

        self.keywordFormat = QtGui.QTextCharFormat(self.base_format)
        self.keywordFormat.setForeground(QtCore.Qt.darkBlue)
        self.keywordFormat.setFontWeight(QtGui.QFont.Bold)
        self.callableFormat = QtGui.QTextCharFormat(self.base_format)
        self.callableFormat.setForeground(QtCore.Qt.darkBlue)
        self.magicFormat = QtGui.QTextCharFormat(self.base_format)
        self.magicFormat.setForeground(QtGui.QColor(224, 128, 0))
        self.qtFormat = QtGui.QTextCharFormat(self.base_format)
        self.qtFormat.setForeground(QtCore.Qt.blue)
        self.qtFormat.setFontWeight(QtGui.QFont.Bold)
        self.selfFormat = QtGui.QTextCharFormat(self.base_format)
        self.selfFormat.setForeground(QtCore.Qt.red)
        # self.selfFormat.setFontItalic(True)
        self.singleLineCommentFormat = QtGui.QTextCharFormat(self.base_format)
        self.singleLineCommentFormat.setForeground(QtCore.Qt.darkGreen)
        self.multiLineStringFormat = QtGui.QTextCharFormat(self.base_format)
        self.multiLineStringFormat.setBackground(
            QtGui.QBrush(QtGui.QColor(127, 127, 255)))
        self.quotationFormat1 = QtGui.QTextCharFormat(self.base_format)
        self.quotationFormat1.setForeground(QtCore.Qt.blue)
        self.quotationFormat2 = QtGui.QTextCharFormat(self.base_format)
        self.quotationFormat2.setForeground(QtCore.Qt.blue)

    def updateRules(self):

        self.rules = []
        self.rules += map(lambda s: (QtCore.QRegExp(r"\b" + s + r"\b"),
                                     self.keywordFormat), self.keywords)

        self.rules.append((QtCore.QRegExp(r"\b[A-Za-z_]+\(.*\)"), self.callableFormat))
        self.rules.append((QtCore.QRegExp(r"\b__[a-z]+__\b"), self.magicFormat))
        self.rules.append((QtCore.QRegExp(r"\bself\b"), self.selfFormat))
        self.rules.append((QtCore.QRegExp(r"\bQ([A-Z][a-z]*)+\b"), self.qtFormat))

        self.rules.append((QtCore.QRegExp(r"#[^\n]*"), self.singleLineCommentFormat))

        self.multiLineStringBegin = QtCore.QRegExp(r'\"\"\"')
        self.multiLineStringEnd = QtCore.QRegExp(r'\"\"\"')

        self.rules.append((QtCore.QRegExp(r'\"[^\n]*\"'), self.quotationFormat1))
        self.rules.append((QtCore.QRegExp(r"'[^\n]*'"), self.quotationFormat2))

    def updateHighlighter(self, font):

        self.updateFonts(font)
        self.updateRules()
        self.setDocument(self.document)


class QtQmlHighlighter(PythonHighlighter):

    keywords = """"
        break  	for  	throw case 	function 	try
        catch 	if 	typeof continue 	in 	var default 	instanceof 	void
        delete 	new 	undefined do 	return 	while  else 	switch 	with
        finally 	this 	 """.split() + \
        ['NaN', 'Infinity', 'undefined', 'print', 'parseInt',
         'parseFloat', 'isNaN', 'isFinite', 'decodeURI',
         'decodeURIComponent', 'encodeURI', 'encodeURIComponent',
         'escape', 'unescape', 'version', 'gc', 'Object',
         'Function', 'Number', 'Boolean', 'String', 'Date', 'Array',
         'RegExp', 'Error', 'EvalError', 'RangeError', 'ReferenceError',
         'SyntaxError', 'TypeError', 'URIError', 'eval', 'Math',
         'Enumeration', 'Variant', 'QObject', 'QMetaObject']

    def __init__(self, edit):
        PythonHighlighter.__init__(self,  edit)

    def updateRules(self):

        self.rules = []
        self.rules += map(lambda s: (QtCore.QRegExp(r"\b" + s + r"\b"),
                                     self.keywordFormat), self.keywords)

        self.rules.append((QtCore.QRegExp(r"\b[A-Za-z_]+\(.*\)"), self.callableFormat))
        # self.rules.append((QtCore.QRegExp(r"\b__[a-z]+__\b"), self.magicFormat))
        self.rules.append((QtCore.QRegExp(r"\bthis\b"), self.selfFormat))
        self.rules.append((QtCore.QRegExp(r"\bQ([A-Z][a-z]*)+\b"), self.qtFormat))

        self.rules.append((QtCore.QRegExp(r"//[^\n]*"), self.singleLineCommentFormat))

        # XXX quick hack to support QtQml syntax
        self.multiLineStringBegin = QtCore.QRegExp(r'/\*')
        self.multiLineStringEnd = QtCore.QRegExp(r'\*/')
        self.multiLineStringFormat = self.singleLineCommentFormat
        self.rules.append((QtCore.QRegExp(r'\"[^\n]*\"'), self.quotationFormat1))
        self.rules.append((QtCore.QRegExp(r"'[^\n]*'"), self.quotationFormat2))
