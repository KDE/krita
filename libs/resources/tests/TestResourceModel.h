/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef TESTRESOURCEMODEL_H
#define TESTRESOURCEMODEL_H

#include <QObject>
#include <QtSql>

class KisResourceLocator;
class TestResourceModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testRowCount();
    void cleanupTestCase();
private:

    QString m_srcLocation;
    QString m_dstLocation;

    KisResourceLocator *m_locator;
};

#endif
