/* This file is part of the KDE project
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
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
#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <KIcon>

class KoStyleThumbnailer;

class KoStyleManager;
class KoParagraphStyle;
class KoCharacterStyle;

class QImage;
class QSignalMapper;
class QSize;

class StylesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Type {
        CharacterStyle,
        ParagraphStyle
    };

    explicit StylesModel(KoStyleManager *styleManager, Type modelType, QObject *parent = 0);
    ~StylesModel();

    virtual QModelIndex index(int row, int column=0, const QModelIndex &parent = QModelIndex()) const;

    virtual int rowCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    /** Specific methods of the StylesModel */

    KoParagraphStyle *paragraphStyleForIndex(const QModelIndex &index) const;
    QModelIndex indexForParagraphStyle(const KoParagraphStyle &style) const;

    KoCharacterStyle *characterStyleForIndex(const QModelIndex &index) const;
    QModelIndex indexForCharacterStyle(const KoCharacterStyle &style) const;

    QImage stylePreview(int row, QSize size = QSize());

    void setStyleManager(KoStyleManager *manager);
    void setStyleThumbnailer(KoStyleThumbnailer *thumbnailer);

    void setCurrentParagraphStyle(int styleId);

private slots:
    void addParagraphStyle(KoParagraphStyle*);
    void addCharacterStyle(KoCharacterStyle*);
    void removeParagraphStyle(KoParagraphStyle*);
    void removeCharacterStyle(KoCharacterStyle*);
    void updateName(int styleId);

protected:
    QList<int> m_styleList; // list of style IDs

private:
    KoStyleManager *m_styleManager;
    KoStyleThumbnailer *m_styleThumbnailer;

    KoParagraphStyle *m_currentParagraphStyle;
    KoCharacterStyle *m_defaultCharacterStyle;
    Type m_modelType;

    KIcon m_paragIcon, m_charIcon;

    QSignalMapper *m_styleMapper;
};

#endif
