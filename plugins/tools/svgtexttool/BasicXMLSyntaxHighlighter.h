/*
 * SPDX-FileCopyrightText: 2015 Dmitry Ivanov
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef BASIC_XML_SYNTAX_HIGHLIGHTER_H
#define BASIC_XML_SYNTAX_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextEdit>

/**
 * A Basic XML syntax highlighter in C++/Qt (subclass of QSyntaxHighlighter). Uses simple
 * regexes to highlight not very complicated XML content.
 *
 * * The primary limitations are:
 * * No specific handling for nested quotes within attributes
 * * No handling for multi-line comments
 *
 * @see https://github.com/d1vanov/basic-xml-syntax-highlighte
 *
 */
class BasicXMLSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    BasicXMLSyntaxHighlighter(QObject * parent);
    BasicXMLSyntaxHighlighter(QTextDocument * parent);
    BasicXMLSyntaxHighlighter(QTextEdit * parent);

    void setFormats();

protected:
    virtual void highlightBlock(const QString & text);

private:
    void highlightByRegex(const QTextCharFormat & format,
                          const QRegExp & regex, const QString & text);

    void setRegexes();


private:
    QTextCharFormat     m_xmlKeywordFormat;
    QTextCharFormat     m_xmlElementFormat;
    QTextCharFormat     m_xmlAttributeFormat;
    QTextCharFormat     m_xmlValueFormat;
    QTextCharFormat     m_xmlCommentFormat;

    QList<QRegExp>      m_xmlKeywordRegexes;
    QRegExp             m_xmlElementRegex;
    QRegExp             m_xmlAttributeRegex;
    QRegExp             m_xmlValueRegex;
    QRegExp             m_xmlCommentRegex;
};

#endif // BASIC_XML_SYNTAX_HIGHLIGHTER_H
