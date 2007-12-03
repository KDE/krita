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

#include "ui_StyleManager.h"

#include <QWidget>
#include <QList>

class KoStyleManager;
class QListWidgetItem;
class KoParagraphStyle;
class KoCharacterStyle;

class StyleManager : public QWidget {
    Q_OBJECT
public:
    StyleManager(QWidget *parent = 0);
    ~StyleManager();

    void setStyleManager(KoStyleManager *sm);

    void setUnit(const KoUnit &unit);

public slots:
    void save();

private slots:
    void setStyle(QListWidgetItem *item, QListWidgetItem *previous);
    void buttonNewPressed();
    void buttonDeletePressed();
    void setStyleName(const QString &name);
    void addParagraphStyle(KoParagraphStyle*);
    void addCharacterStyle(KoCharacterStyle*);
    void removeParagraphStyle(KoParagraphStyle*);
    void removeCharacterStyle(KoCharacterStyle*);

private:
    Ui::StyleManager widget;
    KoStyleManager *m_styleManager;
    QList<KoParagraphStyle*> m_paragraphStyles;

    QMap<int, KoParagraphStyle*> m_alteredParagraphStyles;
    QMap<int, KoCharacterStyle*> m_alteredCharacterStyles;

    bool m_blockSignals;
};

#endif
