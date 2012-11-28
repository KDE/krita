/* This file is part of the KDE project
 * Copyright (C) 2012 Pierre Stirnweiss <pstirnweiss@googlemail.org>
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

#include "DockerStylesComboModel.h"

#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>
#include <KoStyleManager.h>

#include <KDebug>

DockerStylesComboModel::DockerStylesComboModel(QObject *parent) :
    StylesFilteredModelBase(parent),
    m_styleManager(0)
{
}

void DockerStylesComboModel::setInitialUsedStyles(QVector<int> usedStyles)
{
    Q_UNUSED(usedStyles);
    // This is not used yet. Let's revisit this later.

//    m_usedStyles << usedStyles;
//    beginResetModel();
//    createMapping();
//    endResetModel();
}

void DockerStylesComboModel::setStyleManager(KoStyleManager *sm)
{
    Q_ASSERT(sm);
    Q_ASSERT(m_sourceModel);
    if(!sm || !m_sourceModel || m_styleManager == sm) {
        return;
    }
    m_styleManager = sm;
    m_usedStyles.clear();
    m_usedStylesId.clear();

    if (m_sourceModel->stylesType() == AbstractStylesModel::CharacterStyle) {
        KoCharacterStyle *compareStyle;
        foreach(int i, m_styleManager->usedCharacterStyles()) {
            if (!m_usedStylesId.contains(i)) {
                QVector<int>::iterator begin = m_usedStyles.begin();
                compareStyle = m_styleManager->characterStyle(i);
                for ( ; begin != m_usedStyles.end(); ++begin) {
                    if (m_sourceModel->index(*begin, 0, QModelIndex()).internalId() != -1) { //styleNone (internalId=-1) is a virtual style provided only for the UI. it does not exist in KoStyleManager
                        KoCharacterStyle *s = m_styleManager->characterStyle(m_sourceModel->index(*begin, 0, QModelIndex()).internalId());
                        if (QString::localeAwareCompare(compareStyle->name(), s->name()) < 0) {
                            break;
                        }
                    }
                }
                m_usedStyles.insert(begin, m_sourceModel->indexForCharacterStyle(*compareStyle).row());
                m_usedStylesId.append(i);
            }
        }
    }
    else {
        KoParagraphStyle *compareStyle;
        foreach(int i, m_styleManager->usedParagraphStyles()) {
            if (!m_usedStylesId.contains(i)) {
                QVector<int>::iterator begin = m_usedStyles.begin();
                compareStyle = m_styleManager->paragraphStyle(i);
                for ( ; begin != m_usedStyles.end(); ++begin) {
                    if (m_sourceModel->index(*begin, 0, QModelIndex()).internalId() != -1) { //styleNone (internalId=-1) is a virtual style provided only for the UI. it does not exist in KoStyleManager
                        KoParagraphStyle *s = m_styleManager->paragraphStyle(m_sourceModel->index(*begin, 0, QModelIndex()).internalId());
                        if (QString::localeAwareCompare(compareStyle->name(), s->name()) < 0) {
                            break;
                        }
                    }
                }
                m_usedStyles.insert(begin, m_sourceModel->indexForParagraphStyle(*compareStyle).row());
                m_usedStylesId.append(i);
            }
        }
    }
    createMapping();
}

void DockerStylesComboModel::styleApplied(const KoCharacterStyle *style)
{
    if (!m_usedStylesId.contains(style->styleId())) {
        m_usedStylesId.append(style->styleId());
        if (m_sourceModel->stylesType() == AbstractStylesModel::CharacterStyle) {
            QVector<int>::iterator begin = m_usedStyles.begin();
            for ( ; begin != m_usedStyles.end(); ++begin) {
                if (m_sourceModel->index(*begin, 0, QModelIndex()).internalId() != -1) { //styleNone (internalId=-1) is a virtual style provided only for the UI. it does not exist in KoStyleManager
                    KoCharacterStyle *s = m_styleManager->characterStyle(m_sourceModel->index(*begin, 0, QModelIndex()).internalId());
                    if (QString::localeAwareCompare(style->name(), s->name()) < 0) {
                        break;
                    }
                }
            }
            m_usedStyles.insert(begin, m_sourceModel->indexForCharacterStyle(*style).row());
        }
        else {
            QVector<int>::iterator begin = m_usedStyles.begin();
            for ( ; begin != m_usedStyles.end(); ++begin) {
                KoParagraphStyle *s = m_styleManager->paragraphStyle(m_sourceModel->index(*begin, 0, QModelIndex()).internalId());
                if (QString::localeAwareCompare(style->name(), s->name()) < 0) {
                    break;
                }
            }
            m_usedStyles.insert(begin, m_sourceModel->indexForCharacterStyle(*(style)).row());   // We use the ForCharacterStyle variant also for parag styles because the signal exist only in charStyle variant. TODO merge these functions in StylesModel. they use the styleId anyway.
        }
        //we do not reset the model here, as it will mess up the view's visibility. perhaps this is very wrong. to be considered in case we have bugs.
        createMapping();
    }
}

void DockerStylesComboModel::createMapping()
{
    Q_ASSERT(m_sourceModel);
    if (!m_sourceModel || !m_styleManager) {
        return;
    }

    m_proxyToSource.clear();
    m_sourceToProxy.clear();
    m_unusedStyles.clear();

    //Handle the default characterStyle. If provided, the None virtual style is the first style of the model. Its internalId is -1
    if (m_sourceModel->stylesType() == AbstractStylesModel::CharacterStyle) {
        if (m_sourceModel->index(0, 0, QModelIndex()).isValid() && m_sourceModel->index(0, 0, QModelIndex()).internalId() == -1) {
            if (!m_usedStylesId.contains(-1)) {
                m_usedStylesId.prepend(-1);
                m_usedStyles.prepend(0);
            }
        }
    }

    for (int i = 0; i < m_sourceModel->rowCount(QModelIndex()); ++i) {
        QModelIndex index = m_sourceModel->index(i, 0, QModelIndex());
        int id = (int)index.internalId();
        if (!m_usedStylesId.contains(id)) {
            if (m_sourceModel->stylesType() == AbstractStylesModel::ParagraphStyle) {
                KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(id);
                if (paragStyle) {
                    if (!m_unusedStyles.empty()) {
                        QVector<int>::iterator begin = m_unusedStyles.begin();
                        for ( ; begin != m_unusedStyles.end(); ++begin) {
                            KoParagraphStyle *style = m_styleManager->paragraphStyle(m_sourceModel->index(*begin, 0, QModelIndex()).internalId());
                            if (QString::localeAwareCompare(paragStyle->name(), style->name()) < 0) {
                                break;
                            }
                        }
                        m_unusedStyles.insert(begin, i);
                    }
                    else {
                        m_unusedStyles.append(i);
                    }
                }
            }
            else {
                KoCharacterStyle *charStyle = m_styleManager->characterStyle(id);
                if (charStyle) {
                    if (!m_unusedStyles.empty()) {
                        QVector<int>::iterator begin = m_unusedStyles.begin();
                        for ( ; begin != m_unusedStyles.end(); ++begin) {
                            KoCharacterStyle *style = m_styleManager->characterStyle(m_sourceModel->index(*begin, 0, QModelIndex()).internalId());
                            if (QString::localeAwareCompare(charStyle->name(), style->name()) < 0) {
                                break;
                            }
                        }
                        m_unusedStyles.insert(begin, i);
                    }
                    else {
                        m_unusedStyles.append(i);
                    }
                }
            }
        }
    }
    m_proxyToSource << m_usedStyles << m_unusedStyles;
    m_sourceToProxy.fill(-1, m_sourceModel->rowCount((QModelIndex())));
    for (int i = 0; i < m_proxyToSource.count(); ++i) {
        m_sourceToProxy[m_proxyToSource.at(i)] = i;
    }
}
