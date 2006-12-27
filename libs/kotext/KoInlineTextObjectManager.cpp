#include "KoInlineTextObjectManager.h"

#include <QTextCursor>
#include <QPainter>

KoInlineTextObjectManager::KoInlineTextObjectManager()
    : m_lastObjectId(0)
{
}

KoInlineObjectBase *KoInlineTextObjectManager::inlineTextObject(const QTextFormat &format) const {
    int id = format.intProperty(InlineInstanceId);
    if(id <= 0)
        return 0;
    return m_objects.value(id);
}

KoInlineObjectBase *KoInlineTextObjectManager::inlineTextObject(const QTextCursor &cursor) const {
    return inlineTextObject(cursor.charFormat());
}

void KoInlineTextObjectManager::insertInlineObject(QTextCursor &cursor, KoInlineObjectBase *object) {
    QTextCharFormat cf;
    cf.setObjectType(1001);
    cf.setProperty(InlineInstanceId, ++m_lastObjectId);
    cursor.insertText(QString(0xFFFC), cf);
    object->setId(m_lastObjectId);
    m_objects.insert(m_lastObjectId, object);
}
