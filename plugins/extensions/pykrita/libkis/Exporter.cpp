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
#include "Exporter.h"

struct Exporter::Private {
    Private() {}
};

Exporter::Exporter(QObject *parent) 
    : QObject(parent)
    , d(new Private)
{
}

Exporter::~Exporter() 
{
    delete d;
}

Document* Exporter::document() const
{
    return 0;
}

void Exporter::setDocument(Document* value)
{
}


InfoObject* Exporter::exportSetttings() const
{
    return 0;
}

void Exporter::setExportSetttings(InfoObject* value)
{
}




bool Exporter::Export(const QString &filename)
{
    return false;
}



