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
    /// insert an autonumbered footnote
    void insertAutoFootNote();
    /// insert a labeled footnote
    void insertLabeledFootNote(QString label);
    /// insert an autonumbered endnote
    void insertAutoEndNote();
    /// insert a labeled endnote
    void insertLabeledEndNote(QString label);
    /// show the configuration dialog for notes
    void showNotesConfigureDialog();
    /// enable/disable buttons if cursor in notes' body or not
    void updateButtons();


private:
    TableOfContentsConfigure *m_configure;
    SimpleTableOfContentsWidget *m_stocw;
        SimpleFootEndNotesWidget *m_sfenw;
        KoInlineNote *m_note;
    SimpleCitationBibliographyWidget *m_scbw;
};

class KAction;
class QLineEdit;

class LabeledNoteWidget : public QWidget
{
    Q_OBJECT
public:
    LabeledNoteWidget(KAction *action);
    KAction *m_action;
    QLineEdit *m_lineEdit;

signals:
    void triggered(QString label);

private slots:
    void returnPressed();

protected:
    virtual void enterEvent(QEvent *event);
};

#endif // REFERENCESTOOL_H
