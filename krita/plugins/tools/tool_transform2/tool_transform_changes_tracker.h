/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __TOOL_TRANSFORM_CHANGES_TRACKER_H
#define __TOOL_TRANSFORM_CHANGES_TRACKER_H

#include <QObject>

#include "tool_transform_args.h"
#include "transform_transaction_properties.h"


/**
 * This class is supposed to support undo operations for the Transform
 * Tool. Note, that the transform tool is not going to support redo()
 * operations, so we do not use QUndoStack here to not complicate the
 * structure of classes.
 */
class TransformChangesTracker : public QObject
{
    Q_OBJECT
public:
    TransformChangesTracker(TransformTransactionProperties *transaction);

    void commitConfig(const ToolTransformArgs &config);
    void requestUndo();
    void reset();

    bool isEmpty() const;

signals:
    void sigConfigChanged();

private:
    QList<ToolTransformArgs> m_config;
    TransformTransactionProperties *m_transaction;
};

#endif /* __TOOL_TRANSFORM_CHANGES_TRACKER_H */
