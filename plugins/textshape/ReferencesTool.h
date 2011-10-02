/* This file is part of the KDE project
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

#ifndef REFERENCESTOOL_H
#define REFERENCESTOOL_H

#include "TextTool.h"

class KoCanvasBase;
class TableOfContentsConfigure;
class SimpleTableOfContentsWidget;
class SimpleFootEndNotesWidget;
class SimpleCitationBibliographyWidget;
class KoInlineNote;
class QPainter;

/// This tool is the ui for inserting Table of Contents, Citations/bibliography, footnotes, endnotes, index, table of illustrations etc

class ReferencesTool : public TextTool
{
    Q_OBJECT
public:
    ReferencesTool(KoCanvasBase *canvas);

    virtual ~ReferencesTool();

    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();

    virtual void createActions();

protected:
    /// reimplemented from superclass
    virtual QList<QWidget *> createOptionWidgets();

private slots:
    /// insert a table of contents
    void insertTableOfContents();
    /// insert a citation
    void insertCitation();
    /// insert a bibliography
    void insertBibliography();
    /// configure a bibliography
    void configureBibliography();
    /// format the table of contents template
    void formatTableOfContents();
    /// shows the configuration dialog for a ToC
    void showConfigureDialog(QAction *action);
    /// hides the configuration dialog for ToC
    void hideCofigureDialog(int result);
    /// insert a footnote
    void insertFootNote();
    /// insert an endnote
    void insertEndNote();
    /// show the configuration dialog for notes
    void showNotesConfigureDialog();
    /// disable insert notes' buttons when already in notes' body
    void disableButtons(QTextCursor cursor);

private:
    TableOfContentsConfigure *m_configure;
    SimpleTableOfContentsWidget *m_stocw;
        SimpleFootEndNotesWidget *m_sfenw;
        KoInlineNote *m_note;
    SimpleCitationBibliographyWidget *m_scbw;
};

#endif // REFERENCESTOOL_H
