/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2014
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
#ifndef KOFILEDIALOGTESTER_H
#define KOFILEDIALOGTESTER_H

#include <QWidget>

namespace Ui {
class KoFileDialogTester;
}

class KoFileDialogTester : public QWidget
{
    Q_OBJECT
    
public:
    explicit KoFileDialogTester(QWidget *parent = 0);
    ~KoFileDialogTester();
    
private slots:

    void testOpenFile();
    void testOpenFiles();
    void testOpenDirectory();
    void testOpenDirectories();
    void testImportFile();
    void testImportFiles();
    void testImportDirectory();
    void testImportDirectories();
    void testSaveFile();
    void testSaveFiles();

private:
    Ui::KoFileDialogTester *ui;
};

#endif // KOFILEDIALOGTESTER_H
