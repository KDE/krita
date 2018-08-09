/* This file is part of the KDE project
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "TableOfContentsStyleModel.h"

#include "KoStyleManager.h"
#include <KoStyleThumbnailer.h>
#include "KoParagraphStyle.h"
#include "ToCBibGeneratorInfo.h"
#include "KoTableOfContentsGeneratorInfo.h"
#include "kis_assert.h"

#include <QPair>

#include <klocalizedstring.h>

TableOfContentsStyleModel::TableOfContentsStyleModel(const KoStyleManager *manager, KoTableOfContentsGeneratorInfo *info)
    : QAbstractTableModel()
    , m_styleManager(manager)
    , m_styleThumbnailer(new KoStyleThumbnailer())
    , m_tocInfo(info)
{
    Q_ASSERT(manager);
    Q_ASSERT(info);

    m_styleThumbnailer->setThumbnailSize(QSize(250, 48));

    Q_FOREACH (const KoParagraphStyle *style, m_styleManager->paragraphStyles()) {
        m_styleList.append(style->styleId());
        m_outlineLevel.append(getOutlineLevel(style->styleId()));
    }

}

QModelIndex TableOfContentsStyleModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || column > 1) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (row >= m_styleList.count()) {
            return QModelIndex();
        }

        QPair<int, int> *modelValue = new QPair<int, int>(m_styleList[row], m_outlineLevel[row]);
        return createIndex(row, column, modelValue);
    }
    return QModelIndex();
}

int TableOfContentsStyleModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_styleList.count();
    }
    return 0;
}

int TableOfContentsStyleModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return 2;
    }
    return 0;
}

QVariant TableOfContentsStyleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int id = static_cast< QPair<int, int> *>(index.internalPointer())->first;
    if (index.column() == 0) {
        switch (role) {
        case Qt::DisplayRole: {
            return QVariant();
        }
        case Qt::DecorationRole: {
            if (!m_styleThumbnailer) {
                return QPixmap();
            }
            KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(id);
            if (paragStyle) {
                return m_styleThumbnailer->thumbnail(paragStyle);
            }
            break;
        }
        default: break;
        }
    } else {
        KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(id);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paragStyle, QVariant());

        switch (role) {
        case Qt::DisplayRole: {
            if (QVariant(static_cast< QPair<int, int> *>(index.internalPointer())->second).value<int>() == 0) {
                return QVariant(i18n("Disabled"));
            } else {
                return QVariant(static_cast< QPair<int, int> *>(index.internalPointer())->second);
            }
        }
        case Qt::EditRole: {
            return QVariant(static_cast< QPair<int, int> *>(index.internalPointer())->second);
        }
        default: break;
        }
    }
    return QVariant();
}

Qt::ItemFlags TableOfContentsStyleModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }
    if (index.column() == 0) {
        return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
    if (index.column() == 1) {
        return (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }
    return 0;
}

bool TableOfContentsStyleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    static_cast< QPair<int, int> *>(index.internalPointer())->second = value.toInt();
    QAbstractTableModel::setData(index, value, role);
    m_outlineLevel[index.row()] = value.toInt();
    return true;
}

void TableOfContentsStyleModel::saveData()
{
    int row = 0;

    Q_FOREACH (const int styleId, m_styleList) {
        KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(styleId);
        if (paragStyle) {
            setOutlineLevel(styleId, m_outlineLevel[row]);
        }
        row++;
    }
}

int TableOfContentsStyleModel::getOutlineLevel(int styleId)
{
    foreach (const IndexSourceStyles &indexSourceStyles, m_tocInfo->m_indexSourceStyles) {
        foreach (const IndexSourceStyle &indexStyle, indexSourceStyles.styles) {
            if (m_styleManager->paragraphStyle(indexStyle.styleId) && styleId == indexStyle.styleId) {
                return indexSourceStyles.outlineLevel;
            }
        }
    }
    return 0;
}

void TableOfContentsStyleModel::setOutlineLevel(int styleId, int outLineLevel)
{
    //ignore changes to paragraph styles with KoParagraphStyle::OutlineLevel property set.
    //i.e. those considered by KoTableOfContentsGeneratorInfo::m_useOutlineLevel==true
    if (m_styleManager->paragraphStyle(styleId)->hasProperty(KoParagraphStyle::OutlineLevel)) {
        return;
    }

    //check if the outlineLevel has changed
    if (getOutlineLevel(styleId) == outLineLevel) {
        return;
    }

    //now insert the style at the correct place( remove from the old place first and then insert at the new level)
    IndexSourceStyle indexStyleMoved;
    bool styleFound = false;
    int sourceStyleIndex = 0;
    foreach (const IndexSourceStyles &indexSourceStyles, m_tocInfo->m_indexSourceStyles) {
        int index = 0;
        foreach (const IndexSourceStyle &indexStyle, indexSourceStyles.styles) {
            if (styleId == indexStyle.styleId) {
                styleFound = true;
                indexStyleMoved = m_tocInfo->m_indexSourceStyles[sourceStyleIndex].styles.takeAt(index);
                break;
            }
            index++;

            if (styleFound == true) {
                break;
            }
        }
        sourceStyleIndex++;
    }

    //this style is not in the IndexSourceStyles list so fill it
    if (!styleFound) {
        indexStyleMoved.styleId = styleId;
        indexStyleMoved.styleName = m_styleManager->paragraphStyle(styleId)->name();
    }

    //check if IndexSourceStyles are there for this outlineLevel, if not create it
    bool sourceStylePresent = false;
    foreach (const IndexSourceStyles &indexSourceStyles, m_tocInfo->m_indexSourceStyles) {
        if (outLineLevel == indexSourceStyles.outlineLevel) {
            sourceStylePresent = true;
            break;
        }
    }

    if (!sourceStylePresent) {
        IndexSourceStyles indexStyles;
        indexStyles.outlineLevel = outLineLevel;
        m_tocInfo->m_indexSourceStyles.append(indexStyles);
    }

    sourceStyleIndex = 0;
    foreach (const IndexSourceStyles &indexSourceStyles, m_tocInfo->m_indexSourceStyles) {
        if (outLineLevel == indexSourceStyles.outlineLevel) {
            m_tocInfo->m_indexSourceStyles[sourceStyleIndex].styles.append(indexStyleMoved);
            break;
        }
        sourceStyleIndex++;
    }

}

QVariant TableOfContentsStyleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return i18n("Styles");
        } else if (section == 1) {
            return i18n("Level");
        } else {
            return QAbstractTableModel::headerData(section, orientation, role);
        }
    } else {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
}
