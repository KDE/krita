/* This file is part of the Calligra project, made within the KDE community.
 *
 * Copyright (C) 2013 Friedrich W. H. Kossebau <friedrich@kogmbh.com>
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

#ifndef TEXTDOCUMENTSTRUCTUREMODEL_H
#define TEXTDOCUMENTSTRUCTUREMODEL_H

#include <QAbstractItemModel>
#include <QVector>
#include <QHash>
#include <QPointer>

class QTextDocument;
class QTextFrame;
class QTextBlock;

struct NodeData {
    enum Type {Frame, Block};

    Type type;
    union {
        QTextFrame *frame;
        int blockNumber;
    };
    static NodeData fromFrame(QTextFrame *frame);
    static NodeData fromBlock(int blockNumber);
};

class TextDocumentStructureModel : public QAbstractItemModel
{
    Q_OBJECT

    enum Columns {
        nameColumn = 0,
        endColumn
    };

public:
    explicit TextDocumentStructureModel(QObject *parent = 0);
    virtual ~TextDocumentStructureModel();

public: // QAbstractItemModel API
    virtual QModelIndex index(int row, int column, const QModelIndex &parentIndex) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &index) const;
    virtual int columnCount(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool hasChildren(const QModelIndex &parent) const;

public:
    void setTextDocument(QTextDocument *textDocument);

private Q_SLOTS:
    void onContentsChanged();
    void onModelReset();

private:
    int blockIndex(const QTextBlock &block) const;
    int frameIndex(QTextFrame *frame) const;

private:
    QPointer<QTextDocument> m_textDocument;

    mutable QVector<NodeData> m_nodeDataTable;
    mutable QHash<int, int> m_blockNumberTable;
    mutable QHash<QTextFrame *, int> m_frameTable;
};

#endif // TEXTDOCUMENTSTRUCTUREMODEL_H
