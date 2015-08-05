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
#include <kundo2qstack.h>
#include "KisUndoView.h"
#include "KisUndoModel.h"

#ifndef QT_NO_UNDOVIEW

#include <kundo2group.h>
#include <QAbstractItemModel>
#include <QPointer>
#include <QIcon>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QWidgetAction>
#include <QGridLayout>
#include <kis_config.h>



/*!
    \class KisUndoView
    \brief The KisUndoView class displays the contents of a KUndo2QStack.
    \since 4.2

    \ingroup advanced

    KisUndoView is a QListView which displays the list of commands pushed on an undo stack.
    The most recently executed command is always selected. Selecting a different command
    results in a call to KUndo2QStack::setIndex(), rolling the state of the document
    backwards or forward to the new command.

    The stack can be set explicitly with setStack(). Alternatively, a KUndo2Group object can
    be set with setGroup(). The view will then update itself automatically whenever the
    active stack of the group changes.

    \image KisUndoView.png
*/

class KisUndoViewPrivate
{
public:
    KisUndoViewPrivate() :
#ifndef QT_NO_UNDOGROUP
        group(0),
#endif
        model(0) {}

#ifndef QT_NO_UNDOGROUP
    QPointer<KUndo2Group> group;
#endif
    KisUndoModel *model;
    KisUndoView* q;

    void init(KisUndoView* view);
};

void KisUndoViewPrivate::init(KisUndoView* view)
{
    q = view;
    model = new KisUndoModel(q);
    q->setModel(model);
    q->setSelectionModel(model->selectionModel());
}

/*!
    Constructs a new view with parent \a parent.
*/

KisUndoView::KisUndoView(QWidget *parent)
    : QListView(parent)
    , d(new KisUndoViewPrivate)
{
    d->init(this);
}

/*!
    Constructs a new view with parent \a parent and sets the observed stack to \a stack.
*/

KisUndoView::KisUndoView(KUndo2QStack *stack, QWidget *parent)
    : QListView(parent)
    , d(new KisUndoViewPrivate)
{
    d->init(this);
    setStack(stack);
}

#ifndef QT_NO_UNDOGROUP

/*!
    Constructs a new view with parent \a parent and sets the observed group to \a group.

    The view will update itself automatically whenever the active stack of the group changes.
*/

KisUndoView::KisUndoView(KUndo2Group *group, QWidget *parent)
    : QListView(parent)
    , d(new KisUndoViewPrivate)
{
    d->init(this);
    setGroup(group);
}

#endif // QT_NO_UNDOGROUP

/*!
    Destroys this view.
*/

KisUndoView::~KisUndoView()
{
    delete d;

}

/*!
    Returns the stack currently displayed by this view. If the view is looking at a
    KUndo2Group, this the group's active stack.

    \sa setStack() setGroup()
*/

KUndo2QStack *KisUndoView::stack() const
{

    return d->model->stack();
}

/*!
    Sets the stack displayed by this view to \a stack. If \a stack is 0, the view
    will be empty.

    If the view was previously looking at a KUndo2Group, the group is set to 0.

    \sa stack() setGroup()
*/

void KisUndoView::setStack(KUndo2QStack *stack)
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

void KisUndoView::setGroup(KUndo2Group *group)
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
        d->model->setStack(d->group->activeStack());
    } else {
        d->model->setStack(0);
    }
}

/*!
    Returns the group displayed by this view.

    If the view is not looking at group, this function returns 0.

    \sa setGroup() setStack()
*/

KUndo2Group *KisUndoView::group() const
{

    return d->group;
}

#endif // QT_NO_UNDOGROUP

/*!
    \property KisUndoView::emptyLabel
    \brief the label used for the empty state.

    The empty label is the topmost element in the list of commands, which represents
    the state of the document before any commands were pushed on the stack. The default
    is the string "<empty>".
*/

void KisUndoView::setEmptyLabel(const QString &label)
{

    d->model->setEmptyLabel(label);
}

QString KisUndoView::emptyLabel() const
{

    return d->model->emptyLabel();
}

/*!
    \property KisUndoView::cleanIcon
    \brief the icon used to represent the clean state.

    A stack may have a clean state set with KUndo2QStack::setClean(). This is usually
    the state of the document at the point it was saved. KisUndoView can display an
    icon in the list of commands to show the clean state. If this property is
    a null icon, no icon is shown. The default value is the null icon.
*/

void KisUndoView::setCleanIcon(const QIcon &icon)
{

    d->model->setCleanIcon(icon);

}

QIcon KisUndoView::cleanIcon() const
{

    return d->model->cleanIcon();
}

void KisUndoView::setCanvas(KisCanvas2 *canvas) {
    d->model->setCanvas(canvas);
}
void KisUndoView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        QMenu menu(this);
        QAction* action1 = menu.addAction(themedIcon("link"),stack()->useCumulativeUndoRedo()?i18n("Disable Cumulative Undo"):i18n("Enable Cumulative Undo"));
        connect(action1, SIGNAL(triggered()), this, SLOT(toggleCumulativeUndoRedo()));
        QLabel *l = new QLabel("Start merging time");
        QDoubleSpinBox *s = new QDoubleSpinBox();
        s->setToolTip("The amount of time after a merged stroke before merging again");
        s->setRange(3,10);
        s->setValue(stack()->timeT1());
        QGridLayout *g = new QGridLayout();
        g->addWidget(l);
        g->addWidget(s);
        QWidget *w = new QWidget();
        w->setLayout(g);
        w->setVisible(stack()->useCumulativeUndoRedo());
        QWidgetAction* action2 = new QWidgetAction(s);
        action2->setDefaultWidget(w);
        connect(s,SIGNAL(valueChanged(double)),SLOT(setStackT1(double)));

        QLabel *l1 = new QLabel("Group time");
        QDoubleSpinBox *s1 = new QDoubleSpinBox();
        s1->setToolTip("The amount of time every stroke should be \napart from its previous stroke\nto be classified in one group");
        s1->setRange(0.3,s->value());
        s1->setValue(stack()->timeT2());
        QGridLayout *g1 = new QGridLayout();
        g1->addWidget(l1);
        g1->addWidget(s1);
        QWidget *w1 = new QWidget();
        w1->setLayout(g1);
        w1->setVisible(stack()->useCumulativeUndoRedo());
        QWidgetAction* action3 = new QWidgetAction(s1);
        action3->setDefaultWidget(w1);
        connect(s1,SIGNAL(valueChanged(double)),SLOT(setStackT2(double)));

        QLabel *l2 = new QLabel("Split Strokes");
        QSpinBox *s2 = new QSpinBox();
        s2->setToolTip("The number of last strokes which Krita should store separately");
        s2->setRange(1,stack()->undoLimit());
        s2->setValue(stack()->strokesN());
        QGridLayout *g2 = new QGridLayout();
        g1->addWidget(l2);
        g1->addWidget(s2);
        QWidget *w2 = new QWidget();
        w2->setLayout(g2);
        w2->setVisible(stack()->useCumulativeUndoRedo());
        QWidgetAction* action4 = new QWidgetAction(s2);
        action4->setDefaultWidget(w2);
        connect(s2,SIGNAL(valueChanged(int)),SLOT(setStackN(int)));

        menu.addAction(action2);
        menu.addAction(action3);
        menu.addAction(action4);

        menu.exec(event->globalPos());

    }
    else{
    QListView::mousePressEvent(event);
    }
}
void KisUndoView::toggleCumulativeUndoRedo()
{
    stack()->setUseCumulativeUndoRedo(!stack()->useCumulativeUndoRedo() );
    KisConfig cfg;
    cfg.setCumulativeUndoRedo(stack()->useCumulativeUndoRedo());
}
void KisUndoView::setStackT1(double value)
{
    stack()->setTimeT1(value);
    KisConfig cfg;
    cfg.setStackT1(value);
}
void KisUndoView::setStackT2(double value)
{
    stack()->setTimeT2(value);
    KisConfig cfg;
    cfg.setStackT2(value);
}
void KisUndoView::setStackN(int value)
{
    stack()->setStrokesN(value);
    KisConfig cfg;
    cfg.setStackN(value);
}


#endif // QT_NO_UNDOVIEW
