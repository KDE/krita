/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Importer.h"

struct Importer::Private {
    Private() {}
};

Importer::Importer(QObject *parent) 
    : QObject(parent)
    , d(new Private)
{
}

Importer::~Importer() 
{
    delete d;
}

Document* Importer::document() const
{
    return 0;
}

void Importer::setDocument(Document* value)
{
}


InfoObject* Importer::importSettings() const
{
    return 0;
}

void Importer::setImportSettings(InfoObject* value)
{
}




bool Importer::Import(const QString &filename)
{
    return false;
}



