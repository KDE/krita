#include "%{APPNAME}.h"

#include <QTextBlock>
#include <QTextDocument>

#include <KLocale>
#include <KDebug>

%{APPNAME}::%{APPNAME}()
{
	// Add advanced GUI here
}

void %{APPNAME}::finishedWord(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
}

void %{APPNAME}::finishedParagraph(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
}

void %{APPNAME}::checkSection(QTextDocument *document, int startPosition, int endPosition)
{
    m_cursor = QTextCursor(document);
    m_cursor.setPosition(startPosition);
    m_cursor.setPosition(endPosition, QTextCursor::KeepAnchor);
    m_document = document;

    m_startPosition = startPosition;
    m_endPosition = endPosition;
	
	emit startMacro(i18n("%{APPNAME}"));
	
	// Do modifications here
	
	emit stopMacro();
}
