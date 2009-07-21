/* This file is part of the KDE libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>
                 2002 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoFilter.h"

#include <QFile>
#include <kurl.h>
#include <kmimetype.h>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <QStack>
#include "KoFilterManager.h"


class KoFilter::Private
{
public:
};

class KoEmbeddingFilter::Private
{
public:
    /**
     * A stack which keeps track of the current part references.
     * We push one PartState structure for every embedding level.
     */
    QStack<PartState*> partStack;
};

KoFilter::KoFilter(QObject* parent) : QObject(parent), m_chain(0), d(0)
{
}

KoFilter::~KoFilter()
{
    delete d;
}


KoEmbeddingFilter::~KoEmbeddingFilter()
{
    if (d->partStack.count() != 1)
        kWarning() << "Someone messed with the part stack";
    delete d->partStack.pop();
    delete d;
}

int KoEmbeddingFilter::lruPartIndex() const
{
    return d->partStack.top()->m_lruPartIndex;
}

QString KoEmbeddingFilter::mimeTypeByExtension(const QString& extension)
{
    // We need to resort to an ugly hack to determine the mimetype
    // from the extension, as kservicetypefactory.h isn't installed
    KUrl url;
    url.setPath(QString("dummy.%1").arg(extension));
    KMimeType::Ptr m(KMimeType::findByUrl(url, 0, true, true));
    return m->name();
}

KoEmbeddingFilter::KoEmbeddingFilter()
    : KoFilter(),
    d(new Private())
{
    d->partStack.push(new PartState());
}

int KoEmbeddingFilter::embedPart(const QByteArray& from, QByteArray& to,
                                 KoFilter::ConversionStatus& status, const QString& key)
{
    ++(d->partStack.top()->m_lruPartIndex);

    KTemporaryFile tempIn;
    tempIn.open();
    savePartContents(&tempIn);

    KoFilterManager *manager = new KoFilterManager(tempIn.fileName(), from, m_chain);
    status = manager->exportDocument(QString(), to);
    delete manager;

    // Add the part to the current "stack frame", using the number as key
    // if the key string is empty
    PartReference ref(lruPartIndex(), to);
    d->partStack.top()->m_partReferences.insert(key.isEmpty() ? QString::number(lruPartIndex()) : key, ref);

    return lruPartIndex();
}

void KoEmbeddingFilter::startInternalEmbedding(const QString& key, const QByteArray& mimeType)
{
    filterChainEnterDirectory(QString::number(++(d->partStack.top()->m_lruPartIndex)));
    PartReference ref(lruPartIndex(), mimeType);
    d->partStack.top()->m_partReferences.insert(key, ref);
    d->partStack.push(new PartState());
}

void KoEmbeddingFilter::endInternalEmbedding()
{
    if (d->partStack.count() == 1) {
        kError(30500) << "You're trying to endInternalEmbedding more often than you started it" << endl;
        return;
    }
    delete d->partStack.pop();
    filterChainLeaveDirectory();
}

int KoEmbeddingFilter::internalPartReference(const QString& key) const
{
    QMap<QString, PartReference>::const_iterator it = d->partStack.top()->m_partReferences.constFind(key);
    if (it == d->partStack.top()->m_partReferences.constEnd())
        return -1;
    return it.value().m_index;
}

QByteArray KoEmbeddingFilter::internalPartMimeType(const QString& key) const
{
    QMap<QString, PartReference>::const_iterator it = d->partStack.top()->m_partReferences.constFind(key);
    if (it == d->partStack.top()->m_partReferences.constEnd())
        return QByteArray();
    return it.value().m_mimeType;
}

KoEmbeddingFilter::PartReference::PartReference(int index, const QByteArray& mimeType) :
        m_index(index), m_mimeType(mimeType)
{
}

bool KoEmbeddingFilter::PartReference::isValid() const
{
    return m_index != 1 && !m_mimeType.isEmpty();
}

KoEmbeddingFilter::PartState::PartState() : m_lruPartIndex(0)
{
}

void KoEmbeddingFilter::savePartContents(QIODevice*)
{
}

void KoEmbeddingFilter::filterChainEnterDirectory(const QString& directory) const
{
    m_chain->enterDirectory(directory);
}

void KoEmbeddingFilter::filterChainLeaveDirectory() const
{
    m_chain->leaveDirectory();
}

#include <KoFilter.moc>
