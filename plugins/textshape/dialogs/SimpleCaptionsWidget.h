/* This file is part of the KDE project
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
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
#ifndef SIMPLECAPTIONSWIDGET_H
#define SIMPLECAPTIONSWIDGET_H

#include <ui_SimpleCaptionsWidget.h>
#include <KoListStyle.h>

#include <QWidget>
#include <QTextBlock>

class TextTool;
class KoStyleManager;

class SimpleCaptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleCaptionsWidget(QWidget *parent = 0);

public Q_SLOTS:
    void setStyleManager(KoStyleManager *sm);

Q_SIGNALS:
    void doneWithFocus();

private:
    Ui::SimpleCaptionsWidget widget;
    KoStyleManager *m_styleManager;
    bool m_blockSignals;
    bool m_comboboxHasBidiItems;
    QTextBlock m_currentBlock;
    TextTool *m_tool;
};

#endif
