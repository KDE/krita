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

#include <TextDocumentStructureModel.h>

// KDE
#include <QDebug>
// Qt
#include <QTextDocument>
#include <QTextFrame>
#include <QTextBlock>
#include <QTextCursor>

NodeData NodeData::fromBlock(int blockNumber)
{
    NodeData nodeData;
    nodeData.type = Block;
    nodeData.blockNumber = blockNumber;
    return nodeData;
}

NodeData NodeData::fromFrame(QTextFrame *frame)
{
    NodeData nodeData;
    nodeData.type = Frame;
    nodeData.frame = frame;
    return nodeData;
}

TextDocumentStructureModel::TextDocumentStructureModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    connect(this, SIGNAL(modelReset()), SLOT(onModelReset()));
}

TextDocumentStructureModel::~TextDocumentStructureModel()
{
}

int TextDocumentStructureModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);

    return endColumn;
}

int TextDocumentStructureModel::rowCount(const QModelIndex &index) const
{
    qDebug() << "-------------------------- index:" << index << m_textDocument;
    if (!m_textDocument) {
        return 0;
    }

    if (!index.isValid()) {
        // one root frame
        return 1;
    }

    Q_ASSERT(index.internalId() < uint(m_nodeDataTable.count()));

    const NodeData &nodeData = m_nodeDataTable.at(index.internalId());

    if (nodeData.type == NodeData::Frame) {
        QTextFrame *frame = nodeData.frame;

        // count frames and blocks
        int count = 0;
        for (QTextFrame::iterator iterator = frame->begin(); !iterator.atEnd(); ++iterator) {
            ++count;
        }
        return count;
    }

    // should be a block then, no childs for now
    return 0;
}

QVariant TextDocumentStructureModel::data(const QModelIndex &index, int role) const
{
    if (!m_textDocument || ! index.isValid()) {
        return QVariant();
    }

    Q_ASSERT(index.internalId() < uint(m_nodeDataTable.count()));

    const NodeData &nodeData = m_nodeDataTable.at(index.internalId());

    switch (role) {
    case Qt::DisplayRole: {
        if (nodeData.type == NodeData::Frame) {
            QTextFrame *frame = nodeData.frame;
            return QLatin1String(frame->metaObject()->className());
        }
        // else should be a block
        return QLatin1String("Block");
    }
    }

    return QVariant();
}

QModelIndex TextDocumentStructureModel::parent(const QModelIndex &index) const
{
    qDebug() << "-------------------------- index:" << index << m_textDocument;
    if (!m_textDocument || ! index.isValid()) {
        return QModelIndex();
    }

    Q_ASSERT(index.internalId() < uint(m_nodeDataTable.count()));

    const NodeData &nodeData = m_nodeDataTable.at(index.internalId());

    QTextFrame *parentFrame;
    if (nodeData.type == NodeData::Frame) {
        parentFrame = nodeData.frame->parentFrame();
    } else {
        QTextBlock block = m_textDocument->findBlockByNumber(nodeData.blockNumber);
        Q_ASSERT(block.isValid());
        // QTextBlock's API has no option to query the parentframe, so get it via a cursor
        QTextCursor cursor(block);
        parentFrame = cursor.currentFrame();
    }

    if (!parentFrame) {
        return QModelIndex();
    }

    QTextFrame *grandParentFrame = parentFrame->parentFrame();
    // parent is root frame?
    if (!grandParentFrame) {
        Q_ASSERT(parentFrame == m_textDocument->rootFrame());
        return createIndex(0, 0, static_cast<quintptr>(0));
    }

    // find position of parentFrame
    bool posFound = false;
    int row = 0;
    for (QTextFrame::iterator iterator = grandParentFrame->begin(); !iterator.atEnd(); ++iterator) {
        if (iterator.currentFrame() == parentFrame) {
            posFound = true;
            break;
        }
        ++row;
    }
    Q_ASSERT(posFound); Q_UNUSED(posFound);
    return createIndex(row, 0, frameIndex(parentFrame));
}

QModelIndex TextDocumentStructureModel::index(int row, int column, const QModelIndex &parentIndex) const
{
    qDebug() << "-------------------------- row:" << row << "column:" << column << "index:" << parentIndex << m_textDocument;
    if (!m_textDocument) {
        return QModelIndex();
    }

    if (!parentIndex.isValid()) {
        return createIndex(row, column, static_cast<quintptr>(0));
    }

    Q_ASSERT(parentIndex.internalId() < uint(m_nodeDataTable.count()));

    const NodeData &nodeData = m_nodeDataTable.at(parentIndex.internalId());
    // can be only frame for now
    Q_ASSERT(nodeData.type == NodeData::Frame);

    QTextFrame *parentFrame = nodeData.frame;
    int index = -1;
    int count = 0;
    for (QTextFrame::iterator iterator = parentFrame->begin(); !iterator.atEnd(); ++iterator) {
        if (count == row) {
            QTextFrame *frame = iterator.currentFrame();
            if (frame) {
                index = frameIndex(frame);
                break;
            } else {
                QTextBlock block = iterator.currentBlock();
                if (block.isValid()) {
                    index = blockIndex(block);
                    break;
                }
            }
        }
        ++count;
    }

    Q_ASSERT(index != -1);
    return createIndex(row, column, index);
}

bool TextDocumentStructureModel::hasChildren(const QModelIndex &parentIndex) const
{
    qDebug() << "-------------------------- parentIndex:" << parentIndex << m_textDocument;
    if (!m_textDocument) {
        return false;
    }
    // there is one root children
    if (!parentIndex.isValid()) {
        return true;
    }

    Q_ASSERT(parentIndex.internalId() < uint(m_nodeDataTable.count()));

    const NodeData &nodeData = m_nodeDataTable.at(parentIndex.internalId());

    if (nodeData.type == NodeData::Frame) {
        return (! nodeData.frame->begin().atEnd());
    }
    // no children with a block
    return false;
}

void TextDocumentStructureModel::setTextDocument(QTextDocument *textDocument)
{
    if (m_textDocument) {
        m_textDocument->disconnect(this);
    }

    m_textDocument = textDocument;

    if (m_textDocument) {
        connect(m_textDocument, SIGNAL(contentsChanged()), SLOT(onContentsChanged()));
    }

    reset();
}

int TextDocumentStructureModel::blockIndex(const QTextBlock &block) const
{
    int index;

    const int blockNumber = block.blockNumber();
    QHash<int, int>::ConstIterator it = m_blockNumberTable.constFind(blockNumber);
    if (it == m_blockNumberTable.constEnd()) {
        index = m_nodeDataTable.count();
        m_blockNumberTable.insert(blockNumber, index);
        m_nodeDataTable.append(NodeData::fromBlock(blockNumber));
    } else {
        index = it.value();
    }

    return index;
}

int TextDocumentStructureModel::frameIndex(QTextFrame *frame) const
{
    int index;

    QHash<QTextFrame *, int>::ConstIterator it = m_frameTable.constFind(frame);
    if (it == m_frameTable.constEnd()) {
        index = m_nodeDataTable.count();
        m_frameTable.insert(frame, index);
        m_nodeDataTable.append(NodeData::fromFrame(frame));
    } else {
        index = it.value();
    }

    return index;
}

void TextDocumentStructureModel::onContentsChanged()
{
    reset();
}

void TextDocumentStructureModel::onModelReset()
{
    qDebug() << "-------------------------- " << m_textDocument;
    m_nodeDataTable.clear();
    m_blockNumberTable.clear();
    m_frameTable.clear();

    // prefill table with root node
    if (m_textDocument) {
        QTextFrame *rootFrame = m_textDocument->rootFrame();
        m_frameTable.insert(rootFrame, 0);
        m_nodeDataTable.append(NodeData::fromFrame(rootFrame));
    }
}
