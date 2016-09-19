/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KUNDO2COMMANDH_H
#define KUNDO2COMMANDH_H

#include "kundo2magicstring.h"
#include "kundo2commandextradata.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QAction>
#include <QTime>
#include <QVector>

#include <kundo2command.h>
#include <kundo2stack.h>
#include "kritaundo2_export.h"

class QAction;
class KUndo2CommandPrivate;
class KUndo2Group;
class KActionCollection;

class KRITAUNDO2_EXPORT KUndo2Command : public QObject
{

    Q_OBJECT

private:

    KUndo2CommandPrivate *d;
    int timedID;

public:
    explicit KUndo2Command(KUndo2Command *parent = 0);
    explicit KUndo2Command(const KUndo2MagicString &text, KUndo2Command *parent = 0);
    virtual ~KUndo2Command();

    virtual void undo();
    virtual void redo();

    QString actionText() const;
    KUndo2MagicString text() const;
    void setText(const KUndo2MagicString &text);

    virtual int id() const;
    virtual int timedId();
    virtual void setTimedID(int timedID);
    virtual bool mergeWith(const KUndo2Command *other);
    virtual bool timedMergeWith(KUndo2Command *other);

    int childCount() const;
    //const KUndo2Command *child(int index) const;

    bool hasParent();
    virtual void setTime();
    virtual QTime time();
    virtual void setEndTime();
    virtual QTime endTime();

    virtual QVector<KUndo2Command*> mergeCommandsVector();
    virtual bool isMerged();
    virtual void undoMergedCommands();
    virtual void redoMergedCommands();

    /**
     * \return user-defined object associated with the command
     *
     * \see setExtraData()
     */
    KUndo2CommandExtraData* extraData() const;

    /**
     * The user can assign an arbitrary object associated with the
     * command. The \p data object is owned by the command. If you assign
     * the object twice, the first one will be destroyed.
     */
    void setExtraData(KUndo2CommandExtraData *data);

private:
    Q_DISABLE_COPY(KUndo2Command)
    friend class KUndo2QStack;


    bool m_hasParent;
    int m_timedID;

    QTime m_timeOfCreation;
    QTime m_endOfCommand;
    QVector<KUndo2Command*> m_mergeCommandsVector;
};

#endif // KUNDO2COMMANDH_H
