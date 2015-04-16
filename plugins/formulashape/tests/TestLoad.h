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

#ifndef _TESTLOAD_H_
#define _TESTLOAD_H_

#include <QObject>

class TestLoad : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // Token Elements
    void identifierElement_data();
    void identifierElement();
    void numberElement_data();
    void numberElement();
    void operatorElement_data();
    void operatorElement();
    void textElement_data();
    void textElement();
    void spaceElement_data();
    void spaceElement();
    void stringElement_data();
    void stringElement();
    void glyphElement_data();
    void glyphElement();
    

    // General layout elements
    void rowElement_data();
    void rowElement();
    void fracElement_data();
    void fracElement();
    void rootElement_data();
    void rootElement();
    void styleElement_data();
    void styleElement();
    void errorElement_data();
    void errorElement();
    void paddedElement_data();
    void paddedElement();
    void phantomElement_data();
    void phantomElement();
    void fencedElement_data();
    void fencedElement();
    void encloseElement_data();
    void encloseElement();

    // Script and Limit Elements
    void subElement_data();
    void subElement();
    void supElement_data();
    void supElement();
    void subsupElement_data();
    void subsupElement();
    void underElement_data();
    void underElement();
    void overElement_data();
    void overElement();
    void underOverElement_data();
    void underOverElement();
    void multiscriptsElement_data();
    void multiscriptsElement();

    // Tables and Matrices
    void tableElement_data();
    void tableElement();
    void trElement_data();
    void trElement();
//    void labeledtrElement_data();
//    void labeledtrElement();
    void tdElement_data();
    void tdElement();

    // Enlivening expressions
    void actionElement_data();
    void actionElement();
};

#endif // _TESTLOAD_H_
