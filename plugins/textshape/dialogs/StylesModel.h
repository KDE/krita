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
#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <QMultiHash>
#include <QIcon>
#include <QPixmap>
#include <QMap>

class KoStyleThumbnailer;

class KoStyleManager;
class KoParagraphStyle;
class KoCharacterStyle;
class QSignalMapper;
class TextShape;

class StylesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit StylesModel(KoStyleManager *styleManager, bool paragraphMode, QObject *parent = 0);
    ~StylesModel();

    virtual QModelIndex index(int row, int column=0, const QModelIndex &parent = QModelIndex()) const;

    virtual int rowCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    KoParagraphStyle *paragraphStyleForIndex(const QModelIndex &index) const;
    QModelIndex indexForParagraphStyle(const KoParagraphStyle &style) const;

    KoCharacterStyle *characterStyleForIndex(const QModelIndex &index) const;
    QModelIndex indexForCharacterStyle(const KoCharacterStyle &style) const;

    void setStyleManager(KoStyleManager *manager);
    void setStyleThumbnailer(KoStyleThumbnailer *thumbnailer);

public slots:
    /**
        Sets the paragraph style that is to be marked as the 'active' one.
        @param styleId the id from KoParagraphStyle::styleId()
        @param unchanged if true the icon will display the paragraph style in the text has no local modifications.
    */
    void setCurrentParagraphStyle(int styleId, bool unchanged);
    /**
        Sets the character style that is to be marked as the 'active' one.
        @param styleId the id from KoCharacterStyle::styleId()
        @param unchanged if true the icon will display the character style in the text has no local modifications.
    */
    void setCurrentCharacterStyle(int styleId, bool unchanged);

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

    int m_currentParagraphStyle;
    int m_currentCharacterStyle;
    bool m_pureParagraphStyle;
    bool m_pureCharacterStyle;
    bool m_paragraphMode;

    QIcon m_paragIcon, m_charIcon;

    QSignalMapper *m_styleMapper;
    TextShape *m_tmpTextShape;
};

#endif
