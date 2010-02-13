/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "rdf/SemanticStylesheet.h"

#include <QCoreApplication>
#include <kdebug.h>
#include "KoTextEditor.h"

#include "KoDocumentRdf.h"
#include "KoDocument.h"
#include "KoTextDocument.h"

#include "KoChangeTrackerDisabledRAII.h"

QString SemanticStylesheet::TYPE_SYSTEM = "System";
QString SemanticStylesheet::TYPE_USER = "User";

SemanticStylesheet::SemanticStylesheet(const QString& uuid,
                                       const QString& name,
                                       const QString& templateString,
                                       const QString& type, bool isMutable)
        : QObject(QCoreApplication::instance()),
        m_uuid(uuid),
        m_name(name),
        m_templateString(templateString),
        m_type(type),
        m_isMutable(isMutable)
{
}

QString SemanticStylesheet::uuid()
{
    return m_uuid;
}

QString SemanticStylesheet::name()
{
    return m_name;
}

QString SemanticStylesheet::templateString()
{
    return m_templateString;
}

QString SemanticStylesheet::type()
{
    return m_type;
}


bool SemanticStylesheet::isMutable()
{
    return m_isMutable;
}

void SemanticStylesheet::name(const QString& v)
{
    if (m_isMutable) {
        emit nameChanging(this, m_name, v);
        m_name = v;
    }
}

void SemanticStylesheet::templateString(const QString& v)
{
    if (m_isMutable) {
        m_templateString = v;
    }
}


void SemanticStylesheet::format(RdfSemanticItem* obj, KoTextEditor* editor, const QString& xmlid)
{
    kDebug(30015) << "formating obj:" << obj << " name:" << obj->name();
    kDebug(30015) << "xmlid:" << xmlid << " editor:" << editor << " sheet-name:" << name();
    KoDocumentRdf* rdf = obj->DocumentRdf();
    Q_ASSERT(rdf);
    Q_ASSERT(editor);
    Q_ASSERT(rdf->document());
    QPair< int, int > p;
    if (xmlid.size()) {
        p = rdf->findExtent(xmlid);
    } else {
        p = rdf->findExtent(editor);
    }
    int startpos = p.first + 1;
    int endpos = p.second;
    if (!endpos) {
        kDebug(30015) << "format() invalid range, skipping! start:" << startpos << " end:" << endpos;
        return;
    }
    KoTextDocument ktd(editor->document());
    KoChangeTrackerDisabledRAII disableChangeTracker(ktd.changeTracker());
    editor->beginEditBlock();
    editor->setPosition(startpos, QTextCursor::MoveAnchor);
    editor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, endpos - startpos);
    QString oldText = editor->selectedText();
    editor->removeSelectedText();
    editor->setPosition(startpos, QTextCursor::MoveAnchor);
    kDebug(30015) << "formating start:" << startpos << " end:" << endpos;
    kDebug(30015) << "semantic item oldText:" << oldText;
    QString data = templateString();
    QMap< QString, QString > m;
    m["%NAME%"] = obj->name();
    obj->setupStylesheetReplacementMapping(m);

    for (QMap< QString, QString >::iterator mi = m.begin(); mi != m.end(); ++mi) {
        QString k = mi.key();
        QString v = mi.value();
        data = data.replace(k, v);
    }
    // make sure there is something in the replacement other than commas and spaces
    QString tmpstring = data;
    tmpstring = tmpstring.replace(" ", "");
    tmpstring = tmpstring.replace(",", "");
    if (!tmpstring.size()) {
        kDebug(30015) << "stylesheet results in empty data, using name() instead";
        data = name();
    }
    kDebug(30015) << "Updating with new formatting:" << data;
    editor->insertText(data);
    editor->setPosition(startpos, QTextCursor::MoveAnchor);
    editor->endEditBlock();
}

