/*
 * SPDX-FileCopyrightText: 2015 Dmitry Ivanov
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef BASIC_XML_SYNTAX_HIGHLIGHTER_H
#define BASIC_XML_SYNTAX_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextEdit>
#include <QRegularExpression>

/**
 * A Basic XML syntax highlighter in C++/Qt (subclass of QSyntaxHighlighter). Uses simple
 * regexes to highlight not very complicated XML content.
 *
 * * The primary limitations are:
 * * No specific handling for nested quotes within attributes
 * * No handling for multi-line comments
 *
 * @see https://github.com/d1vanov/basic-xml-syntax-highlighter
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
    void highlightBlock(const QString &text) override;

private:
    void highlightByRegex(const QTextCharFormat & format,
                          const QRegularExpression & regex, const QString & text);

    void setRegexes();


private:
    QTextCharFormat     m_xmlKeywordFormat;
    QTextCharFormat     m_xmlElementFormat;
    QTextCharFormat     m_xmlAttributeFormat;
    QTextCharFormat     m_xmlValueFormat;
    QTextCharFormat     m_xmlCommentFormat;

    QList<QRegularExpression> m_xmlKeywordRegexes;
    QRegularExpression        m_xmlElementRegex;
    QRegularExpression        m_xmlAttributeRegex;
    QRegularExpression        m_xmlValueRegex;
    QRegularExpression        m_xmlCommentRegex;
};

#endif // BASIC_XML_SYNTAX_HIGHLIGHTER_H
