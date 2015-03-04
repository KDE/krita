/* This file is part of the KDE project
   Copyright 2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _TESTLAYOUT_H_
#define _TESTLAYOUT_H_

#include <QObject>

class TestLayout : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // Token Elements
    void identifierElement_data();
    void identifierElement();

    // General Layout Elements
    void fencedElement_data();
    void fencedElement();
};

#endif // _TESTLAYOUT_H_
