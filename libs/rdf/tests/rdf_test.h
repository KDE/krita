/* This file is part of the KDE project
 *
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */



#ifndef RDF_TEST_H
#define RDF_TEST_H

#include <QObject>

class KoTextEditor;
class KoDocumentRdf;
class QString;

class RdfTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreateMarkers();
    void testFindMarkers();
    void testFindByName();
    void testEditAndFindMarkers(); // XXX: implement
    void testRemoveMarkers();


private:

    // @return the xml id for the inserted item
    QString insertTableWithSemItem(KoTextEditor &editor,
                          KoDocumentRdf &rdfDoc,
                          const QString name);
};

#endif // RDF_TEST_H
