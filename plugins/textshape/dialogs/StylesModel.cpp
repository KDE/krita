/* This file is part of the KDE project
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
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
#include "StylesModel.h"
#include "TextTool.h"

#include <QSet>
#include <QDebug>
#include <QSignalMapper>
#include <QTextLayout>
#include <QTextBlock>

#include <KoStyleManager.h>
#include <KoStyleThumbnailer.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

#include <KIcon>
#include <KoTextBlockData.h>
#include <KoParagraphStyle.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextDocumentLayout.h>
#include <KoZoomHandler.h>

#include <KDebug>

#include <QTextLayout>


StylesModel::StylesModel(KoStyleManager *manager, bool paragraphMode, QObject *parent)
    : QAbstractListModel(parent),
      m_styleManager(0),
      m_styleThumbnailer(0),
      m_currentParagraphStyle(0),
      m_currentCharacterStyle(0),
      m_pureParagraphStyle(true),
      m_pureCharacterStyle(true),
      m_paragraphMode(paragraphMode),
      m_styleMapper(new QSignalMapper(this)),
      m_tmpTextShape(0)
{
    setStyleManager(manager);
    m_paragIcon = KIcon("kotext-paragraph");
    m_charIcon = KIcon("kotext-character");
    connect(m_styleMapper, SIGNAL(mapped(int)), this, SLOT(updateName(int)));
}

StylesModel::~StylesModel()
{
    delete m_styleThumbnailer;
    delete m_tmpTextShape;
}

QModelIndex StylesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0)
        return QModelIndex();

    if (! parent.isValid()) {
        if (row >= m_styleList.count())
            return QModelIndex();
        return createIndex(row, column, m_styleList[row]);
    }
    return QModelIndex();
}


int StylesModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_styleList.count();
    return 0;
}

QVariant StylesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int id = (int) index.internalId();
    switch (role) {
    case Qt::DisplayRole: {
        return QVariant();
        KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(id);
        if (paragStyle)
            return paragStyle->name();
        KoCharacterStyle *characterStyle =  m_styleManager->characterStyle(id);
        if (characterStyle)
            return characterStyle->name();
        break;
    }
    case Qt::DecorationRole: {
        KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(id);
        if (paragStyle) {
            return m_styleThumbnailer->thumbnail(paragStyle);
        }
        KoCharacterStyle *characterStyle =  m_styleManager->characterStyle(id);
        if (characterStyle) {
            return m_styleThumbnailer->thumbnail(characterStyle);
        }
        break;
    }
    default: break;
    };
    return QVariant();
}

Qt::ItemFlags StylesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void StylesModel::setCurrentParagraphStyle(int styleId, bool unchanged)
{
    if (m_currentParagraphStyle == styleId && unchanged == m_pureParagraphStyle)
        return;
    m_currentParagraphStyle = styleId;
    m_pureParagraphStyle = unchanged;
}

void StylesModel::setCurrentCharacterStyle(int styleId, bool unchanged)
{
    if (m_currentCharacterStyle == styleId && unchanged == m_pureCharacterStyle)
        return;
    m_currentCharacterStyle = styleId;
    m_pureCharacterStyle = unchanged;
}

KoParagraphStyle *StylesModel::paragraphStyleForIndex(const QModelIndex &index) const
{
    return m_styleManager->paragraphStyle(index.internalId());
}

QModelIndex StylesModel::indexForParagraphStyle(const KoParagraphStyle &style) const
{
    if (&style) {
        return createIndex(m_styleList.indexOf(style.styleId()), 0, style.styleId());;
    }
    else {
        return QModelIndex();
    }
}

KoCharacterStyle *StylesModel::characterStyleForIndex(const QModelIndex &index) const
{
    return m_styleManager->characterStyle(index.internalId());
}

QModelIndex StylesModel::indexForCharacterStyle(const KoCharacterStyle &style) const
{
    if (&style) {
        return createIndex(m_styleList.indexOf(style.styleId()), 0, style.styleId());
    }
    else {
        return QModelIndex();
    }
}

void StylesModel::setStyleManager(KoStyleManager *sm)
{
    if (sm == m_styleManager)
        return;
    if (m_styleManager) {
        disconnect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
        disconnect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
    }
    m_styleManager = sm;
    if (m_styleManager == 0) {
        return;
    }

    if (m_paragraphMode) {
        foreach(KoParagraphStyle *style, m_styleManager->paragraphStyles())
            addParagraphStyle(style);
        connect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
        connect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
    } else {
        foreach(KoCharacterStyle *style, m_styleManager->characterStyles())
            addCharacterStyle(style);
        connect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
        connect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
    }
}

void StylesModel::setStyleThumbnailer(KoStyleThumbnailer *thumbnailer)
{
    m_styleThumbnailer = thumbnailer;
}

// called when the stylemanager adds a style
void StylesModel::addParagraphStyle(KoParagraphStyle *style)
{
    Q_ASSERT(style);
    m_styleList.append(style->styleId());
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager adds a style
void StylesModel::addCharacterStyle(KoCharacterStyle *style)
{
    foreach(KoParagraphStyle *ps, m_styleManager->paragraphStyles()) {
        if (style->styleId() == ps->characterStyle()->styleId()) {
            return;
        }
    }
    Q_ASSERT(style);
    m_styleList.append(style->styleId());
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager removes a style
void StylesModel::removeParagraphStyle(KoParagraphStyle *style)
{
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager removes a style
void StylesModel::removeCharacterStyle(KoCharacterStyle *style)
{
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

void StylesModel::updateName(int styleId)
{
    // TODO, no idea how to do this more correct for children...
    int row = m_styleList.indexOf(styleId);
    if (row >= 0) {
        QModelIndex index = createIndex(row, 0, styleId);
        emit dataChanged(index, index);
    }
}
