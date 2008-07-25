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

#ifndef CHARACTERGENERAL_H
#define CHARACTERGENERAL_H

#include "ui_CharacterGeneral.h"

#include <QWidget>

class KoCharacterStyle;
class CharacterStyleOptions;
class CharacterDecorations;
class CharacterHighlighting;
class KFontChooser;

class CharacterGeneral : public QWidget {
    Q_OBJECT
public:
    explicit CharacterGeneral(QWidget *parent = 0);

    void setStyle(KoCharacterStyle *style);

public slots:
    void save();

    void switchToGeneralTab();

signals:
    void nameChanged(const QString &name);

private slots:
    void setName(const QString &name);

private:
    Ui::CharacterGeneral widget;
    bool m_blockSignals;

    CharacterStyleOptions *m_styleOptions;
    CharacterDecorations *m_characterDecorations;
    CharacterHighlighting *m_characterHighlighting;
    KFontChooser *m_fontChooser;

    KoCharacterStyle *m_style;
};

#endif
