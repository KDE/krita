/* This file is part of the KDE project
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.org>
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

#include <KoStyleThumbnailer.h>
#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

#include <QImage>
#include <QList>
#include <QSignalMapper>

#include <KLocale>
#include <KDebug>

StylesModel::StylesModel(KoStyleManager *manager, Type modelType, QObject *parent)
    : QAbstractListModel(parent),
      m_styleManager(0),
      m_styleThumbnailer(0),
      m_currentParagraphStyle(0),
      m_defaultCharacterStyle(0),
      m_modelType(modelType),
      m_styleMapper(new QSignalMapper(this)),
      m_provideStyleNone(false)
{
    setStyleManager(manager);
    //Create a default characterStyle for the preview of "None" character style
    if (m_modelType == StylesModel::CharacterStyle) {
        m_defaultCharacterStyle = new KoCharacterStyle();
        m_defaultCharacterStyle->setStyleId(-1);
        m_defaultCharacterStyle->setName(i18n("None"));
        m_defaultCharacterStyle->setFontPointSize(12);

        m_provideStyleNone = true;
    }

    connect(m_styleMapper, SIGNAL(mapped(int)), this, SLOT(updateName(int)));
}

StylesModel::~StylesModel()
{
    delete m_currentParagraphStyle;
    delete m_defaultCharacterStyle;
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

    int id = (int)index.internalId();
    switch (role){
    case Qt::DisplayRole: {
        return QVariant();
    }
    case Qt::DecorationRole: {
        if (!m_styleThumbnailer) {
            return QPixmap();
        }
        if (m_modelType == StylesModel::ParagraphStyle) {
            KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(id);
            if (paragStyle) {
                return m_styleThumbnailer->thumbnail(paragStyle);
            }
            if (!paragStyle && m_draftParStyleList.contains(id)) {
                return m_styleThumbnailer->thumbnail(m_draftParStyleList[id]);
            }
        }
        else {
            KoCharacterStyle *usedStyle = 0;
            if (id == -1) {
                usedStyle = static_cast<KoCharacterStyle*>(m_currentParagraphStyle);
                if (!usedStyle) {
                    usedStyle = m_defaultCharacterStyle;
                }
                usedStyle->setName(i18n("None"));
                if (usedStyle->styleId() >= 0) { //if the styleId is -1, we are using the default character style
                    usedStyle->setStyleId(-usedStyle->styleId()); //this style is not managed by the styleManager but its styleId will be used in the thumbnail cache as part of the key.
                }
                return m_styleThumbnailer->thumbnail(usedStyle);
            }
            else {
                usedStyle = m_styleManager->characterStyle(id);
                if (usedStyle) {
                    return m_styleThumbnailer->thumbnail(usedStyle, m_currentParagraphStyle);
                }
                if (!usedStyle && m_draftCharStyleList.contains(id)) {
                    return m_styleThumbnailer->thumbnail(m_draftCharStyleList[id]);
                }
            }
        }
        break;
    }
    case Qt::SizeHintRole: {
        return QVariant(QSize(250, 48));
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

void StylesModel::setCurrentParagraphStyle(int styleId)
{
    if (!m_styleManager || m_currentParagraphStyle == m_styleManager->paragraphStyle(styleId) || !m_styleManager->paragraphStyle(styleId)) {
        return; //TODO do we create a default paragraphStyle? use the styleManager default?
    }
    if (m_currentParagraphStyle) {
        delete m_currentParagraphStyle;
        m_currentParagraphStyle = 0;
    }
    m_currentParagraphStyle = m_styleManager->paragraphStyle(styleId)->clone();
}

void StylesModel::setProvideStyleNone(bool provide)
{
    if (m_modelType == StylesModel::CharacterStyle) {
        m_provideStyleNone = provide;
    }
}

KoParagraphStyle *StylesModel::paragraphStyleForIndex(const QModelIndex &index) const
{
    if (m_draftParStyleList.contains(index.internalId()))
        return m_draftParStyleList[index.internalId()];
    return m_styleManager->paragraphStyle(index.internalId());
}

QModelIndex StylesModel::indexForParagraphStyle(const KoParagraphStyle &style) const
{
    if (&style) {
        QModelIndex index = createIndex(m_styleList.indexOf(style.styleId()), 0, style.styleId());
        return index;
    }
    else {
        return QModelIndex();
    }
}

KoCharacterStyle *StylesModel::characterStyleForIndex(const QModelIndex &index) const
{
    if (m_draftCharStyleList.contains(index.internalId()))
        return m_draftCharStyleList[index.internalId()];
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

QImage StylesModel::stylePreview(int row, QSize size)
{
    if (!m_styleManager || !m_styleThumbnailer) {
        return QImage();
    }
    if (m_modelType == StylesModel::ParagraphStyle) {
        KoParagraphStyle *usedStyle = 0;
        usedStyle = m_styleManager->paragraphStyle(index(row).internalId());
        if (usedStyle) {
            return m_styleThumbnailer->thumbnail(usedStyle, size);
        }
        if (!usedStyle && m_draftParStyleList.contains(index(row).internalId())) {
            return m_styleThumbnailer->thumbnail(m_draftParStyleList[index(row).internalId()], size);
        }
    }
    else {
        KoCharacterStyle *usedStyle = 0;
        if (index(row).internalId() == -1) {
            usedStyle = static_cast<KoCharacterStyle*>(m_currentParagraphStyle);
            if (!usedStyle) {
                usedStyle = m_defaultCharacterStyle;
            }
            usedStyle->setName(i18n("None"));
            if (usedStyle->styleId() >= 0) {
                usedStyle->setStyleId(-usedStyle->styleId()); //this style is not managed by the styleManager but its styleId will be used in the thumbnail cache as part of the key.
            }
            return m_styleThumbnailer->thumbnail(usedStyle, m_currentParagraphStyle, size);
        }
        else {
            usedStyle = m_styleManager->characterStyle(index(row).internalId());
            if (usedStyle) {
                return m_styleThumbnailer->thumbnail(usedStyle, m_currentParagraphStyle, size);
            }
            if (!usedStyle && m_draftCharStyleList.contains(index(row).internalId())) {
                return m_styleThumbnailer->thumbnail(m_draftCharStyleList[index(row).internalId()],m_currentParagraphStyle, size);
            }
        }
    }
    return QImage();
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

    if (m_modelType == StylesModel::ParagraphStyle) {
        updateParagraphStyles();
        connect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
        connect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
    } else {
        updateCharacterStyles();
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
    QList<int>::iterator begin = m_styleList.begin();
    int index = 0;
    for ( ; begin != m_styleList.end(); ++begin) {
        KoParagraphStyle *s = m_styleManager->paragraphStyle(*begin);;
        if (!s && m_draftParStyleList.contains(*begin))
            s = m_draftParStyleList[*begin];
        // s should be found as the manager and the m_styleList should be in sync
        Q_ASSERT(s);
        if (QString::localeAwareCompare(style->name(), s->name()) < 0) {
            break;
        }
        ++index;
    }
    beginInsertRows(QModelIndex(), index, index);
    m_styleList.insert(begin, style->styleId());
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
    endInsertRows();
}

bool sortParagraphStyleByName(KoParagraphStyle *style1, KoParagraphStyle *style2)
{
    Q_ASSERT(style1);
    Q_ASSERT(style2);
    return QString::localeAwareCompare(style1->name(), style2->name()) < 0;
}

void StylesModel::updateParagraphStyles()
{
    Q_ASSERT(m_styleManager);

    beginResetModel();
    m_styleList.clear();

    QList<KoParagraphStyle *> styles = m_styleManager->paragraphStyles();
    qSort(styles.begin(), styles.end(), sortParagraphStyleByName);

    foreach(KoParagraphStyle *style, styles) {
        m_styleList.append(style->styleId());
        m_styleMapper->setMapping(style, style->styleId());
        connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
    }

    endResetModel();
}

// called when the stylemanager adds a style
void StylesModel::addCharacterStyle(KoCharacterStyle *style)
{
    Q_ASSERT(style);
    // find the place where we need to insert the style
    QList<int>::iterator begin = m_styleList.begin();
    int index = 0;
    // the None style should also be the first one so only start after it
    if (begin != m_styleList.end() && *begin == -1) {
        ++begin;
        ++index;
    }
    for ( ; begin != m_styleList.end(); ++begin) {
        KoCharacterStyle *s = m_styleManager->characterStyle(*begin);;
        if (!s && m_draftCharStyleList.contains(*begin))
            s = m_draftCharStyleList[*begin];
        // s should be found as the manager and the m_styleList should be in sync
        Q_ASSERT(s);
        if (QString::localeAwareCompare(style->name(), s->name()) < 0) {
            break;
        }
        ++index;
    }
    beginInsertRows(QModelIndex(), index, index);
    m_styleList.insert(index, style->styleId());
    endInsertRows();
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

bool sortCharacterStyleByName(KoCharacterStyle *style1, KoCharacterStyle *style2)
{
    Q_ASSERT(style1);
    Q_ASSERT(style2);
    return QString::localeAwareCompare(style1->name(), style2->name()) < 0;
}

void StylesModel::updateCharacterStyles()
{
    Q_ASSERT(m_styleManager);

    beginResetModel();
    m_styleList.clear();

    if (m_provideStyleNone && m_styleManager->paragraphStyles().count()) {
        m_styleList.append(-1);
    }

    QList<KoCharacterStyle *> styles = m_styleManager->characterStyles();
    qSort(styles.begin(), styles.end(), sortCharacterStyleByName);

    foreach(KoCharacterStyle *style, styles) {
        m_styleList.append(style->styleId());
        m_styleMapper->setMapping(style, style->styleId());
        connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
    }

    endResetModel();
}

// called when the stylemanager removes a style
void StylesModel::removeParagraphStyle(KoParagraphStyle *style)
{
    int row = m_styleList.indexOf(style->styleId());
    beginRemoveRows(QModelIndex(), row, row);
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
    m_styleList.removeAt(row);
    endRemoveRows();
}

// called when the stylemanager removes a style
void StylesModel::removeCharacterStyle(KoCharacterStyle *style)
{
    int row = m_styleList.indexOf(style->styleId());
    beginRemoveRows(QModelIndex(), row, row);
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
    m_styleList.removeAt(row);
    endRemoveRows();
}

void StylesModel::updateName(int styleId)
{
    // updating the name of a style can mean that the style needs to be moved inside the list to keep the sort order.
    int oldIndex = m_styleList.indexOf(styleId);
    if (oldIndex >= 0) {
        int newIndex = 0;
        if (m_modelType == StylesModel::ParagraphStyle) {
            KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(styleId);
            if (!paragStyle && m_draftParStyleList.contains(styleId))
                paragStyle = m_draftParStyleList.value(styleId);
            if (paragStyle) {
                m_styleThumbnailer->removeFromCache(paragStyle);

                QList<int>::iterator begin = m_styleList.begin();
                for ( ; begin != m_styleList.end(); ++begin) {
                    // don't test again the same style
                    if (*begin == styleId) {
                        continue;
                    }
                    KoParagraphStyle *s = m_styleManager->paragraphStyle(*begin);
                    if (!s && m_draftParStyleList.contains(*begin))
                        s = m_draftParStyleList[*begin];
                    // s should be found as the manager and the m_styleList should be in sync
                    Q_ASSERT(s);
                    if (QString::localeAwareCompare(paragStyle->name(), s->name()) < 0) {
                        break;
                    }
                    ++newIndex;
                }
                if (oldIndex != newIndex) {
                    // beginMoveRows needs the index where it would be placed when it is still in the old position
                    // so add one when newIndex > oldIndex 
                    beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), newIndex > oldIndex ? newIndex + 1 : newIndex);
                    m_styleList.removeAt(oldIndex);
                    m_styleList.insert(newIndex, styleId);
                    endMoveRows();
                }
            }
        }
        else {
            KoCharacterStyle *characterStyle = m_styleManager->characterStyle(styleId);
            if (!characterStyle && m_draftCharStyleList.contains(styleId))
                characterStyle = m_draftCharStyleList[styleId];
            if (characterStyle) {
                m_styleThumbnailer->removeFromCache(characterStyle);

                QList<int>::iterator begin = m_styleList.begin();
                if (begin != m_styleList.end() && *begin == -1) {
                    ++begin;
                    ++newIndex;
                }
                for ( ; begin != m_styleList.end(); ++begin) {
                    // don't test again the same style
                    if (*begin == styleId) {
                        continue;
                    }
                    KoCharacterStyle *s = m_styleManager->characterStyle(*begin);
                    if (!s && m_draftCharStyleList.contains(*begin))
                        s = m_draftCharStyleList[*begin];
                    // s should be found as the manager and the m_styleList should be in sync
                    Q_ASSERT(s);
                    if (QString::localeAwareCompare(characterStyle->name(), s->name()) < 0) {
                        break;
                    }
                    ++newIndex;
                }
                if (oldIndex != newIndex) {
                    // beginMoveRows needs the index where it would be placed when it is still in the old position
                    // so add one when newIndex > oldIndex 
                    beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), newIndex > oldIndex ? newIndex + 1 : newIndex);
                    m_styleList.removeAt(oldIndex);
                    m_styleList.insert(newIndex, styleId);
                    endMoveRows();
                }
            }
        }
    }
}

QModelIndex StylesModel::firstStyleIndex()
{
    return createIndex(m_styleList.indexOf(m_styleList.at(0)), 0, m_styleList.at(0));
}

QList<int> StylesModel::StyleList()
{
    return m_styleList;
}

QHash<int, KoParagraphStyle *> StylesModel::draftParStyleList()
{
    return m_draftParStyleList;
}

QHash<int, KoCharacterStyle *> StylesModel::draftCharStyleList()
{
    return m_draftCharStyleList;
}

void StylesModel::addDraftParagraphStyle(KoParagraphStyle *style)
{
    style->setStyleId(-(m_draftParStyleList.count()+1));
    m_draftParStyleList.insert(style->styleId(), style);
    addParagraphStyle(style);
}

void StylesModel::addDraftCharacterStyle(KoCharacterStyle *style)
{
    if (m_draftCharStyleList.count() == 0) // we have a character style "m_defaultCharacterStyle" with style id -1 in style model.
        style->setStyleId(-(m_draftCharStyleList.count()+2));
    else
        style->setStyleId(-(m_draftCharStyleList.count()+1));
    m_draftCharStyleList.insert(style->styleId(), style);
    addCharacterStyle(style);
}

void StylesModel::clearDraftStyles()
{
    foreach(KoParagraphStyle *style, m_draftParStyleList.values()) {
        removeParagraphStyle(style);
    }
    m_draftParStyleList.clear();
    foreach(KoCharacterStyle *style, m_draftCharStyleList.values()) {
        removeCharacterStyle(style);
    }
    m_draftCharStyleList.clear();
}
