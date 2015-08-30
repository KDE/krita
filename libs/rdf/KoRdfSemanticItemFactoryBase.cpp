/* This file is part of the Calligra project, made with-in the KDE community

   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2013 Friedrich W. H. Kossebau <kossebau@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoRdfSemanticItemFactoryBase.h"

class Q_DECL_HIDDEN KoRdfSemanticItemFactoryBase::Private
{
public:
    Private(const QString &identifier)
            : id(identifier) {
    }

    const QString id;
};

KoRdfSemanticItemFactoryBase::KoRdfSemanticItemFactoryBase(const QString &id)
        : d(new Private(id))
{
}

KoRdfSemanticItemFactoryBase::~KoRdfSemanticItemFactoryBase()
{
    delete d;
}

QString KoRdfSemanticItemFactoryBase::id() const
{
    return d->id;
}
