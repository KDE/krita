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
#ifndef SIMPLETABLEWIDGET_H
#define SIMPLETABLEWIDGET_H

#include <ui_SimpleTableWidget.h>
#include <KoListStyle.h>
#include <KoBorder.h>

#include <QWidget>
#include <QTextBlock>

class TextTool;
class KoStyleManager;
class KoTableCellStyle;
class KoColor;

class SimpleTableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleTableWidget(TextTool *tool, QWidget *parent = 0);

public Q_SLOTS:
    void setStyleManager(KoStyleManager *sm);
    void emitTableBorderDataUpdated(int i = 0);
    void restartPainting();
    void setBorderColor(const KoColor &);

Q_SIGNALS:
    void doneWithFocus();
    void tableBorderDataUpdated(const KoBorder::BorderData &);

private:
    void fillBorderButton(const QColor &color);

    Ui::SimpleTableWidget widget;
    KoStyleManager *m_styleManager;
    bool m_blockSignals;
    bool m_comboboxHasBidiItems;
    QTextBlock m_currentBlock;
    TextTool *m_tool;
    QList<KoTableCellStyle *> m_cellStyles; // we only fill out the top borderdata for the previews
    int m_lastStyleEmitted;
};

#endif
