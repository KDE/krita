/* This file is part of the KDE project
* SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
*
* SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTSHAPEBACKGROUNDCOMMAND_H
#define TESTSHAPEBACKGROUNDCOMMAND_H

#include <QObject>

class TestShapeBackgroundCommand : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void refCounting();
};

#endif // TESTSHAPEBACKGROUNDCOMMAND_H
