/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTSEGMENTTYPECOMMAND_H
#define TESTSEGMENTTYPECOMMAND_H

#include <QObject>

class TestSegmentTypeCommand : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void changeToCurve();
    void changeToLine();
};

#endif // TESTSEGMENTTYPECOMMAND_H
