/* This file is part of the KDE project
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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
#ifndef SIMPLETABLEOFCONTENTSWIDGET_H
#define SIMPLETABLEOFCONTENTSWIDGET_H

#include <ui_SimpleTableOfContentsWidget.h>

#include <QWidget>
#include <QTextBlock>
#include <QList>

class ReferencesTool;
class KoStyleManager;
class KoTableOfContentsGeneratorInfo;
class TableOfContentsPreview;
class QSignalMapper;
class TableOfContentsTemplate;

class SimpleTableOfContentsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleTableOfContentsWidget(ReferencesTool *tool, QWidget *parent = 0);
    ~SimpleTableOfContentsWidget();

public Q_SLOTS:
    void setStyleManager(KoStyleManager *sm);
    void prepareTemplateMenu();
    void pixmapReady(int templateId);

Q_SIGNALS:
    void doneWithFocus();

private Q_SLOTS:
    void applyTemplate(int templateId);
    void insertCustomToC();

private:
    Ui::SimpleTableOfContentsWidget widget;
    KoStyleManager *m_styleManager;
    bool m_blockSignals;
    QTextBlock m_currentBlock;
    QList<KoTableOfContentsGeneratorInfo *> m_templateList;
    //each template in the template list will have have a previewGenerator that will be deleted after preview is generated
    QList<TableOfContentsPreview *> m_previewGenerator;
    ReferencesTool *m_referenceTool;
    QSignalMapper *m_signalMapper;
    TableOfContentsTemplate *m_templateGenerator;
};

#endif
