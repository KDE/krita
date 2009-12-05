#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <QTextCursor>

#include <KoTextEditingPlugin.h>

class QTextDocument;

class %{APPNAME} : public KoTextEditingPlugin
{
public:
    %{APPNAME}();

    void finishedWord(QTextDocument *document, int cursorPosition);
    void finishedParagraph(QTextDocument *document, int cursorPosition);
    void checkSection(QTextDocument *document, int startPosition, int endPosition);

private:
    QTextCursor m_cursor;
    QTextDocument *m_document;
    int m_startPosition;
    int m_endPosition;
};

#endif
