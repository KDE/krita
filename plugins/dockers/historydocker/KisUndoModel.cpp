/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
#include "KisUndoModel.h"
#include <klocalizedstring.h>

KisUndoModel::KisUndoModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_blockOutgoingHistoryChange = false;
    m_stack = 0;
    m_canvas = 0;
    m_sel_model = new QItemSelectionModel(this, this);
    connect(m_sel_model, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(setStackCurrentIndex(QModelIndex)));
    m_empty_label = i18n("<empty>");
}

QItemSelectionModel *KisUndoModel::selectionModel() const
{
    return m_sel_model;
}

KUndo2QStack *KisUndoModel::stack() const
{
    return m_stack;
}

void KisUndoModel::setStack(KUndo2QStack *stack)
{
    if (m_stack == stack)
        return;

    if (m_stack != 0) {
        disconnect(m_stack, SIGNAL(canRedoChanged(bool)), this, SLOT(stackChanged()));
        disconnect(m_stack, SIGNAL(cleanChanged(bool)), this, SLOT(stackChanged()));
        disconnect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(stackChanged()));
        disconnect(m_stack, SIGNAL(destroyed(QObject*)), this, SLOT(stackDestroyed(QObject*)));
        disconnect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(addImage(int)));
    }

    m_stack = stack;

    if (m_stack != 0) {
        connect(m_stack, SIGNAL(canRedoChanged(bool)), this, SLOT(stackChanged()));
        connect(m_stack, SIGNAL(cleanChanged(bool)), this, SLOT(stackChanged()));
        connect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(stackChanged()));
        connect(m_stack, SIGNAL(destroyed(QObject*)), this, SLOT(stackDestroyed(QObject*)));
        connect(m_stack, SIGNAL(indexChanged(int)), this, SLOT(addImage(int)));
    }

    stackChanged();
}

void KisUndoModel::stackDestroyed(QObject *obj)
{
    if (obj != m_stack)
        return;
    m_stack = 0;

    stackChanged();
}

void KisUndoModel::stackChanged()
{
    beginResetModel();
    endResetModel();

    m_blockOutgoingHistoryChange = true;
    m_sel_model->setCurrentIndex(selectedIndex(), QItemSelectionModel::ClearAndSelect);
    m_blockOutgoingHistoryChange = false;
}

void KisUndoModel::setStackCurrentIndex(const QModelIndex &index)
{
    if (m_blockOutgoingHistoryChange)
        return;

    if (m_stack == 0)
        return;

    if (index == selectedIndex())
        return;

    if (index.column() != 0)
        return;

    m_stack->setIndex(index.row());
}

QModelIndex KisUndoModel::selectedIndex() const
{
    return m_stack == 0 ? QModelIndex() : createIndex(m_stack->index(), 0);
}

QModelIndex KisUndoModel::index(int row, int column, const QModelIndex &parent) const
{
    if (m_stack == 0)
        return QModelIndex();

    if (parent.isValid())
        return QModelIndex();

    if (column != 0)
        return QModelIndex();

    if (row < 0 || row > m_stack->count())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex KisUndoModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int KisUndoModel::rowCount(const QModelIndex &parent) const
{
    if (m_stack == 0)
        return 0;

    if (parent.isValid())
        return 0;

    return m_stack->count() + 1;
}

int KisUndoModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant KisUndoModel::data(const QModelIndex &index, int role) const
{
    if (m_stack == 0){
        return QVariant();
    }

    if (index.column() != 0){
        return QVariant();
    }

    if (index.row() < 0 || index.row() > m_stack->count()){
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (index.row() == 0){
            return m_empty_label;
        }
        KUndo2Command* currentCommand = const_cast<KUndo2Command*>(m_stack->command(index.row() - 1));
        return currentCommand->isMerged()?m_stack->text(index.row() - 1)+"(Merged)":m_stack->text(index.row() - 1);
    }
    else if (role == Qt::DecorationRole) {
        if (index.row() > 0) {
            const KUndo2Command* currentCommand = m_stack->command(index.row() - 1);
            if (m_imageMap.contains(currentCommand)) {
                return m_imageMap[currentCommand];
            }
        }
    }
    return QVariant();
}

QString KisUndoModel::emptyLabel() const
{
    return m_empty_label;
}

void KisUndoModel::setEmptyLabel(const QString &label)
{
    m_empty_label = label;
    stackChanged();
}

void KisUndoModel::setCleanIcon(const QIcon &icon)
{
    m_clean_icon = icon;
    stackChanged();
}

QIcon KisUndoModel::cleanIcon() const
{
    return m_clean_icon;
}

void KisUndoModel::setCanvas(KisCanvas2 *canvas)
{
    m_canvas = canvas;
}

void KisUndoModel::addImage(int idx)
{

    if (m_stack == 0 || m_stack->count() == 0) {
        return;
    }

    const KUndo2Command* currentCommand = m_stack->command(idx-1);
    if (m_stack->count() == idx && !m_imageMap.contains(currentCommand)) {
        KisImageWSP historyImage = m_canvas->image();
        KisPaintDeviceSP paintDevice = historyImage->projection();
        QImage image = paintDevice->createThumbnail(32, 32, 1,
                                                    KoColorConversionTransformation::internalRenderingIntent(),
                                                    KoColorConversionTransformation::internalConversionFlags());
        m_imageMap[currentCommand] = image;
    }

    QList<const KUndo2Command*> list;

    for (int i = 0; i < m_stack->count(); ++i) {
        list << m_stack->command(i);
    }

    for (QMap<const KUndo2Command*, QImage>:: iterator it = m_imageMap.begin(); it != m_imageMap.end();) {
        if (!list.contains(it.key())) {
            it = m_imageMap.erase(it);
        }
        else {
            ++it;
        }
    }
}
bool KisUndoModel::checkMergedCommand(int index)
{
    Q_UNUSED(index);
    return false;
}
