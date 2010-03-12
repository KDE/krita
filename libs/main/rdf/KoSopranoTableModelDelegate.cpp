/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoSopranoTableModelDelegate.h"
#include "KoSopranoTableModel.h"

#include "KoDocumentRdf.h"
#include "../KoDocument.h"

#include <klocale.h>
#include <QComboBox>

KoSopranoTableModelDelegate::KoSopranoTableModelDelegate(QObject *parent)
        : QStyledItemDelegate(parent)
{
}

QWidget *KoSopranoTableModelDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QComboBox *comboBox = new QComboBox(parent);
    if (index.column() == KoSopranoTableModel::ColObjType) {
        comboBox->addItem(i18n("URI"));
        comboBox->addItem(i18n("Literal"));
        comboBox->addItem(i18n("Blank"));
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(emitCommitData()));
    return comboBox;
}

void KoSopranoTableModelDelegate::setEditorData(QWidget *editor,
        const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!comboBox) {
        return QStyledItemDelegate::setEditorData(editor, index);
    }
    int pos = comboBox->findText(index.model()->data(index).toString(),
                                 Qt::MatchExactly);
    comboBox->setCurrentIndex(pos);
}

void KoSopranoTableModelDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!comboBox) {
        return QStyledItemDelegate::setModelData(editor, model, index);
    }
    model->setData(index, comboBox->currentText());
}

void KoSopranoTableModelDelegate::emitCommitData()
{
    emit commitData(qobject_cast<QWidget *>(sender()));
}
