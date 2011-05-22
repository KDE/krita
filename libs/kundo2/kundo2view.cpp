/* This file is part of the KDE project
 * Copyright (C) 2010 Matus Talcik <matus.talcik@gmail.com>
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
/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include "kundo2stack.h"
#include "kundo2view.h"
#include "kundo2model.h"
#include "kundo2group.h"

#ifndef QT_NO_UNDOVIEW

#include "qundogroup.h"
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qpointer.h>
#include <QtGui/qicon.h>


/*!
    \class KUndo2View
    \brief The KUndo2View class displays the contents of a KUndo2QStack.
    \since 4.2

    \ingroup advanced

    KUndo2View is a QListView which displays the list of commands pushed on an undo stack.
    The most recently executed command is always selected. Selecting a different command
    results in a call to KUndo2QStack::setIndex(), rolling the state of the document
    backwards or forward to the new command.

    The stack can be set explicitly with setStack(). Alternatively, a QUndoGroup object can
    be set with setGroup(). The view will then update itself automatically whenever the
    active stack of the group changes.

    \image KUndo2View.png
*/

class KUndo2ViewPrivate
{
public:
    KUndo2ViewPrivate() :
#ifndef QT_NO_UNDOGROUP
        group(0),
#endif
        model(0) {}

#ifndef QT_NO_UNDOGROUP
    QPointer<KUndo2Group> group;
#endif
    KUndo2Model *model;
    KUndo2View* q;

    void init(KUndo2View* view);
};

void KUndo2ViewPrivate::init(KUndo2View* view)
{
    q = view;
    model = new KUndo2Model(q);
    q->setModel(model);
    q->setSelectionModel(model->selectionModel());
}

/*!
    Constructs a new view with parent \a parent.
*/

KUndo2View::KUndo2View(QWidget *parent) : QListView(parent), d(new KUndo2ViewPrivate)
{
    d->init(this);
}

/*!
    Constructs a new view with parent \a parent and sets the observed stack to \a stack.
*/

KUndo2View::KUndo2View(KUndo2QStack *stack, QWidget *parent) : QListView(parent), d(new KUndo2ViewPrivate)
{
    d->init(this);
    setStack(stack);
}

#ifndef QT_NO_UNDOGROUP

/*!
    Constructs a new view with parent \a parent and sets the observed group to \a group.

    The view will update itself autmiatically whenever the active stack of the group changes.
*/

KUndo2View::KUndo2View(KUndo2Group *group, QWidget *parent) : QListView(parent), d(new KUndo2ViewPrivate)
{
    d->init(this);
    setGroup(group);
}

#endif // QT_NO_UNDOGROUP

/*!
    Destroys this view.
*/

KUndo2View::~KUndo2View() {

}

/*!
    Returns the stack currently displayed by this view. If the view is looking at a
    QUndoGroup, this the group's active stack.

    \sa setStack() setGroup()
*/

KUndo2QStack *KUndo2View::stack() const
{

    return d->model->stack();
}

/*!
    Sets the stack displayed by this view to \a stack. If \a stack is 0, the view
    will be empty.

    If the view was previously looking at a QUndoGroup, the group is set to 0.

    \sa stack() setGroup()
*/

void KUndo2View::setStack(KUndo2QStack *stack)
{

#ifndef QT_NO_UNDOGROUP
    setGroup(0);
#endif
    d->model->setStack(stack);
}

#ifndef QT_NO_UNDOGROUP

/*!
    Sets the group displayed by this view to \a group. If \a group is 0, the view will
    be empty.

    The view will update itself autmiatically whenever the active stack of the group changes.

    \sa group() setStack()
*/

void KUndo2View::setGroup(KUndo2Group *group)
{


    if (d->group == group)
        return;

    if (d->group != 0) {
        disconnect(d->group, SIGNAL(activeStackChanged(KUndo2QStack*)),
                d->model, SLOT(setStack(KUndo2QStack*)));
    }

    d->group = group;

    if (d->group != 0) {
        connect(d->group, SIGNAL(activeStackChanged(KUndo2QStack*)),
                d->model, SLOT(setStack(KUndo2QStack*)));
        d->model->setStack((KUndo2QStack *)d->group->activeStack());
    } else {
        d->model->setStack(0);
    }
}

/*!
    Returns the group displayed by this view.

    If the view is not looking at group, this function returns 0.

    \sa setGroup() setStack()
*/

KUndo2Group *KUndo2View::group() const
{

    return d->group;
}

#endif // QT_NO_UNDOGROUP

/*!
    \property KUndo2View::emptyLabel
    \brief the label used for the empty state.

    The empty label is the topmost element in the list of commands, which represents
    the state of the document before any commands were pushed on the stack. The default
    is the string "<empty>".
*/

void KUndo2View::setEmptyLabel(const QString &label)
{

    d->model->setEmptyLabel(label);
}

QString KUndo2View::emptyLabel() const
{

    return d->model->emptyLabel();
}

/*!
    \property KUndo2View::cleanIcon
    \brief the icon used to represent the clean state.

    A stack may have a clean state set with KUndo2QStack::setClean(). This is usually
    the state of the document at the point it was saved. KUndo2View can display an
    icon in the list of commands to show the clean state. If this property is
    a null icon, no icon is shown. The default value is the null icon.
*/

void KUndo2View::setCleanIcon(const QIcon &icon)
{

    d->model->setCleanIcon(icon);

}

QIcon KUndo2View::cleanIcon() const
{

    return d->model->cleanIcon();
}

#include "kundo2view.moc"

#endif // QT_NO_UNDOVIEW
