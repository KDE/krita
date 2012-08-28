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
#include "KoSemanticStylesheet.h"

#include <QCoreApplication>
#include <kdebug.h>

#include "KoDocumentRdf.h"
#include "../KoDocument.h"
#include <KoTextDocument.h>
#include <KoTextEditor.h>

#include "KoChangeTrackerDisabledRAII.h"

class KoSemanticStylesheetPrivate
{
public:
    QString m_uuid;
    QString m_name;
    QString m_templateString;
    QString m_type;
    bool m_isMutable;

    KoSemanticStylesheetPrivate(const QString &uuid, const QString &name, const QString &templateString,
                                const QString &type = "System", bool isMutable = false)
        :
        m_uuid(uuid),
        m_name(name),
        m_templateString(templateString),
        m_type(type),
        m_isMutable(isMutable)
        {}
};


KoSemanticStylesheet::KoSemanticStylesheet(const QString &uuid,
                                       const QString &name,
                                       const QString &templateString,
                                       const QString &type, bool isMutable)
    : QObject(QCoreApplication::instance())
    , d (new KoSemanticStylesheetPrivate (uuid,name,templateString,type,isMutable))
{
}

KoSemanticStylesheet::~KoSemanticStylesheet()
{
    delete d;
}

QString KoSemanticStylesheet::uuid() const
{
    return d->m_uuid;
}

QString KoSemanticStylesheet::name() const
{
    return d->m_name;
}

QString KoSemanticStylesheet::templateString() const
{
    return d->m_templateString;
}

QString KoSemanticStylesheet::type() const
{
    return d->m_type;
}


bool KoSemanticStylesheet::isMutable() const
{
    return d->m_isMutable;
}

void KoSemanticStylesheet::name(const QString &v)
{
    if (d->m_isMutable) {
        emit nameChanging(hKoSemanticStylesheet(this), d->m_name, v);
        d->m_name = v;
    }
}

void KoSemanticStylesheet::templateString(const QString &v)
{
    if (d->m_isMutable) {
        d->m_templateString = v;
    }
}


void KoSemanticStylesheet::format(hKoRdfSemanticItem obj, KoTextEditor *editor, const QString &xmlid)
{
    Q_ASSERT(obj);
    Q_ASSERT(editor);
    kDebug(30015) << "formating obj:" << obj << " name:" << obj->name();
    kDebug(30015) << "xmlid:" << xmlid << " editor:" << editor << " sheet-name:" << name();
    const KoDocumentRdf *rdf = obj->documentRdf();
    Q_ASSERT(rdf);
    Q_ASSERT(editor);
    QPair<int, int> p;
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
    if (editor->hasSelection()) {
        editor->deleteChar(); // deletes the selection
    }
    editor->setPosition(startpos, QTextCursor::MoveAnchor);
    kDebug(30015) << "formating start:" << startpos << " end:" << endpos;
    kDebug(30015) << "semantic item oldText:" << oldText;
    QString data = templateString();
    QMap<QString, QString> m;
    m["%NAME%"] = obj->name();
    obj->setupStylesheetReplacementMapping(m);

    for (QMap<QString, QString>::iterator mi = m.begin(); mi != m.end(); ++mi) {
        QString k = mi.key();
        QString v = mi.value();
        data = data.replace(k, v);
    }
    // make sure there is something in the replacement other than commas and spaces
    QString tmpstring = data;
    tmpstring = tmpstring.remove(' ');
    tmpstring = tmpstring.remove(',');
    if (!tmpstring.size()) {
        kDebug(30015) << "stylesheet results in empty data, using name() instead";
        data = name();
    }
    kDebug(30015) << "Updating with new formatting:" << data;
    editor->insertText(data);
    editor->setPosition(startpos, QTextCursor::MoveAnchor);
    editor->endEditBlock();
}

QString KoSemanticStylesheet::stylesheetTypeSystem()
{
    return "System";
}

QString KoSemanticStylesheet::stylesheetTypeUser()
{
    return "User";
}

