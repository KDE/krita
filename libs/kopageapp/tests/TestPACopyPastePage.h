/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option ) any later version.
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

#ifndef TESTPACOPYPASTEPAGE_H
#define TESTPACOPYPASTEPAGE_H

#include <QObject>
#include <QList>
#include <QPointF>

class QMimeData;
class QPoint;
class MockDocument;
class KoPAPageBase;

class TestPACopyPastePage : public QObject
{
    Q_OBJECT
private:
    void copyAndPaste( MockDocument * doc, QList<KoPAPageBase *> & pages, KoPAPageBase * after );
    QMimeData * copy( MockDocument * doc, QList<KoPAPageBase *> & pages );
    void paste( MockDocument * doc, QMimeData * data, KoPAPageBase * after );
    void addShape( KoPAPageBase * page );

    QPointF m_pos;

private Q_SLOTS:
    void copyPasteMultiplePages();
    void copyPasteSinglePage();
    void copyPasteSingleMasterPage();
    void copyPasteMultipleMasterPages();
    void copyPasteMixedPages();
};

#endif /* TESTPACOPYPASTEPAGE_H */
