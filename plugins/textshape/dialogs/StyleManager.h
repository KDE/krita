/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <ui_StyleManager.h>

#include <QWidget>

class StylesModel;
class KoStyleManager;
class KoStyleThumbnailer;
class KoParagraphStyle;
class KoCharacterStyle;

class QModelIndex;

class StyleManager : public QWidget
{
    Q_OBJECT
public:
    StyleManager(QWidget *parent = 0);
    ~StyleManager();

    void setStyleManager(KoStyleManager *sm);

    void setUnit(const KoUnit &unit);

    //Check that the new name of style is unique or not
    bool checkUniqueStyleName();

public slots:
    void save();
    void setParagraphStyle(KoParagraphStyle *style);
    void setCharacterStyle(KoCharacterStyle *style, bool canDelete = false);
    bool unappliedStyleChanges();

private slots:
    void currentStyleChanged();
    void addParagraphStyle(KoParagraphStyle*);
    void addCharacterStyle(KoCharacterStyle*);
    void removeParagraphStyle(KoParagraphStyle*);
    void removeCharacterStyle(KoCharacterStyle*);
    void slotStyleSelected(QModelIndex index);
    void buttonNewPressed();
    void tabChanged(int index);

private:
    Ui::StyleManager widget;
    KoStyleManager *m_styleManager;

    QMap<int, KoParagraphStyle*> m_alteredParagraphStyles;
    QMap<int, KoCharacterStyle*> m_alteredCharacterStyles;
    QMap<int, KoParagraphStyle*> m_draftParagraphStyles;
    QMap<int, KoCharacterStyle*> m_draftCharacterStyles;

    StylesModel *m_paragraphStylesModel;
    StylesModel *m_characterStylesModel;
    KoStyleThumbnailer *m_thumbnailer;
    KoParagraphStyle *m_selectedParagStyle;
    KoCharacterStyle *m_selectedCharStyle;

    bool m_blockSignals;
    bool m_blockStyleChangeSignals;
    bool m_unappliedStyleChanges;
    bool m_currentStyleChanged;
};

#endif
