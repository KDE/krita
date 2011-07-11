/* This file is part of the KDE project
 * Copyright (C) 2011 Brijesh Patel <brijesh3105@gmail.com>
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
#include "NotesConfigurationDialog.h"

#include <KAction>

#include <QWidget>

NotesConfigurationDialog::NotesConfigurationDialog(QTextDocument *doc, QWidget *parent)
        : QDialog(parent),
          document(doc)
{
    widget.setupUi(this);

    connect(widget.footnote,SIGNAL(toggled(bool)),this,SLOT(footnoteSetup(bool)));
    connect(widget.endnote,SIGNAL(toggled(bool)),this,SLOT(endnoteSetup(bool)));

}

/*void NotesConfigurationDialog::setNotesConfiguration()
{

}

KoOdfNotesConfiguration *NotesConfigurationDialog::notesConfiguration()
{

}*/

void NotesConfigurationDialog::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

void NotesConfigurationDialog::footnoteSetup(bool on)
{
    widget.location_footnote->setEnabled(on);
    if(on) {
        widget.numStyleCombo->setCurrentIndex(0);
        widget.paragraphCombo->setCurrentIndex(7);
        widget.pageCombo->setCurrentIndex(7);
        widget.textareaCombo->setCurrentIndex(8);
        widget.noteareaCombo->setCurrentIndex(9);
    }
}

void NotesConfigurationDialog::endnoteSetup(bool on)
{
    widget.location_endnote->setEnabled(on);
    widget.dockWidget_5->setHidden(on);
    widget.beginAtCombo->setDisabled(on);
    if(on) {
        widget.numStyleCombo->setCurrentIndex(3);
        widget.paragraphCombo->setCurrentIndex(3);
        widget.pageCombo->setCurrentIndex(8);
        widget.textareaCombo->setCurrentIndex(5);
        widget.noteareaCombo->setCurrentIndex(6);
    }
}

#include <NotesConfigurationDialog.moc>
