/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTSHAPEGROUPCOMMAND_H
#define TESTSHAPEGROUPCOMMAND_H

#include <QObject>

class KoShapeGroup;
class KUndo2Command;
class MockShape;

class TestShapeGroupCommand : public QObject
{
    Q_OBJECT
public:
    TestShapeGroupCommand();
    ~TestShapeGroupCommand() override;
private Q_SLOTS:
    void init();
    void cleanup();
    void testToplevelGroup();
    void testToplevelGroupWithExistingTransform();
    void testToplevelGroupTransformLater();

    void testToplevelGroupWithNormalization();

    void testZIndexFromNowhere();
    void testZIndexFromNowhereReordered();
    void testZIndexFromSameGroup();

    void testUngrouping();

private:
    KoShapeGroup *toplevelGroup, *sublevelGroup, *strokeGroup;
    KUndo2Command *cmd1, *cmd2, *strokeCmd;
    MockShape *toplevelShape1, *toplevelShape2, *toplevelShape3, *toplevelShape4;
    MockShape *sublevelShape1, *sublevelShape2;
    MockShape *extraShape1, *extraShape2;
    MockShape *strokeShape1, *strokeShape2;
};

#endif // TESTSHAPEGROUPCOMMAND_H
