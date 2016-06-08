/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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
#ifndef SIMPLECHARACTERWIDGET_H
#define SIMPLECHARACTERWIDGET_H

#include <ui_SimpleCharacterWidget.h>
#include <KoListStyle.h>

#include <QWidget>
#include <QTextBlock>

class TextTool;
class KoStyleManager;
class KoCharacterStyle;
class KoStyleThumbnailer;
class DockerStylesComboModel;
class StylesDelegate;
class StylesModel;

class SimpleCharacterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleCharacterWidget(TextTool *tool, QWidget *parent = 0);
    virtual ~SimpleCharacterWidget();

    void setInitialUsedStyles(QVector<int> list);

public Q_SLOTS:
    void setStyleManager(KoStyleManager *sm);
    void setCurrentFormat(const QTextCharFormat &format, const QTextCharFormat &refBlockCharFormat);
    void setCurrentBlockFormat(const QTextBlockFormat &format);
    void slotCharacterStyleApplied(const KoCharacterStyle *style);

private Q_SLOTS:
    void fontFamilyActivated(int index);
    void fontSizeActivated(int index);
    void styleSelected(int index);
    void styleSelected(const QModelIndex &index);
    void slotShowStyleManager(int index);

Q_SIGNALS:
    void doneWithFocus();
    void characterStyleSelected(KoCharacterStyle *);
    void newStyleRequested(const QString &name);
    void showStyleManager(int styleId);

private:
    void clearUnsetProperties(QTextFormat &format);

    Ui::SimpleCharacterWidget widget;
    KoStyleManager *m_styleManager;
    bool m_blockSignals;
    bool m_comboboxHasBidiItems;
    int m_lastFontFamilyIndex;
    int m_lastFontSizeIndex;
    TextTool *m_tool;
    QTextCharFormat m_currentCharFormat;
    QTextBlockFormat m_currentBlockFormat;
    KoStyleThumbnailer *m_thumbnailer;
    StylesModel *m_stylesModel;
    DockerStylesComboModel *m_sortedStylesModel;
    StylesDelegate *m_stylesDelegate;
};

#endif
