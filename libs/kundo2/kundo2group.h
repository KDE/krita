/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef KUNDO2GROUP_H
#define KUNDO2GROUP_H

#include <QObject>
#include <QString>

#include "kundo2_export.h"

class KUndo2GroupPrivate;
class KUndo2QStack;
class QAction;

#ifndef QT_NO_UNDOGROUP

class KUNDO2_EXPORT KUndo2Group : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KUndo2Group)

public:
    explicit KUndo2Group(QObject *parent = 0);
    ~KUndo2Group();

    void addStack(KUndo2QStack *stack);
    void removeStack(KUndo2QStack *stack);
    QList<KUndo2QStack*> stacks() const;
    KUndo2QStack *activeStack() const;

#ifndef QT_NO_ACTION
    QAction *createUndoAction(QObject *parent) const;
    QAction *createRedoAction(QObject *parent) const;
#endif // QT_NO_ACTION
    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;
    bool isClean() const;

public Q_SLOTS:
    void undo();
    void redo();
    void setActiveStack(KUndo2QStack *stack);

Q_SIGNALS:
    void activeStackChanged(KUndo2QStack *stack);
    void indexChanged(int idx);
    void cleanChanged(bool clean);
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void undoTextChanged(const QString &undoActionText);
    void redoTextChanged(const QString &redoActionText);

private:
    // from QUndoGroupPrivate
    KUndo2QStack *m_active;
    QList<KUndo2QStack*> m_stack_list;

    Q_DISABLE_COPY(KUndo2Group)
};

#endif // QT_NO_UNDOGROUP

#endif // KUNDO2GROUP_H
