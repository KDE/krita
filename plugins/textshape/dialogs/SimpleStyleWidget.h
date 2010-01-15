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
#ifndef SIMPLESTYLEWIDGET_H
#define SIMPLESTYLEWIDGET_H

#include <ui_SimpleStyleWidget.h>
#include <KoListStyle.h>

#include <QWidget>
#include <QTextBlock>

class TextTool;
class KoStyleManager;

class SimpleStyleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleStyleWidget(TextTool *tool, QWidget *parent = 0);

public slots:
    void setCurrentBlock(const QTextBlock &block);
    void setStyleManager(KoStyleManager *sm);
    void setCurrentFormat(const QTextCharFormat& format);

signals:
    void doneWithFocus();

private slots:
    void listStyleChanged(int row);
    void directionChangeRequested();
    void applyAgainPressed();

private:
    enum DirectionButtonState {
        LTR,
        RTL,
        Auto
    };

    void updateDirection(DirectionButtonState state);
    void fillListsCombobox();

    Ui::SimpleStyleWidget widget;
    KoStyleManager *m_styleManager;
    bool m_blockSignals;
    QTextBlock m_currentBlock;
    TextTool *m_tool;
    DirectionButtonState m_directionButtonState;
    KoListStyle::Style m_quickApplyListStyle;
};

#endif
