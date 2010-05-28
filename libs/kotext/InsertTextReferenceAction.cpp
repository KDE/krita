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

#include "InsertTextReferenceAction_p.h"
#include "KoTextLocator.h"
#include "KoTextReference.h"
#include "KoInlineTextObjectManager.h"

#include <KoCanvasBase.h>

#include <KLocale>
#include <KMessageBox>
#include <KPageDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>

InsertTextReferenceAction::InsertTextReferenceAction(KoCanvasBase *canvas, const KoInlineTextObjectManager *manager)
        : InsertInlineObjectActionBase(canvas, i18n("Text Reference")),
        m_manager(manager)
{
}

KoInlineObject *InsertTextReferenceAction::createInlineObject()
{
    const QList<KoTextLocator*> textLocators = m_manager->textLocators();
    if (textLocators.isEmpty()) {
        KMessageBox::information(m_canvas->canvasWidget(), i18n("Please create an index to reference first."));
        return 0;
    }

    QWidget *widget = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(widget);
    widget->setLayout(lay);
    lay->setMargin(0);

    QLabel *label = new QLabel(i18n("Select the index you want to reference"), widget);
    lay->addWidget(label);
    QStringList selectionList;
    foreach(KoTextLocator* locator, textLocators)
        selectionList << locator->word() + '(' + QString::number(locator->pageNumber()) + ')';
    QListWidget *list = new QListWidget(widget);
    lay->addWidget(list);
    list->addItems(selectionList);

    KPageDialog dialog(m_canvas->canvasWidget());
    dialog.setCaption(i18n("%1 Options", i18n("Text Reference"))); // reuse the text passed in the constructor
    dialog.addPage(widget, QString());

    KoVariable *variable = 0;
    if (dialog.exec() == KPageDialog::Accepted && list->currentRow() >= 0) {
        KoTextLocator *locator = textLocators[list->currentRow()];
        Q_ASSERT(locator);
        variable = new KoTextReference(locator->id());
    }
    return variable;
}
