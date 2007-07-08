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

#include "KoRulerController.h"
#include "KoText.h"
#include "styles/KoParagraphStyle.h"

#include <KoCanvasResourceProvider.h>

#include <KDebug>

#include <QVariant>
#include <QTextOption>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextLayout>
#include <QTextCursor>

#include <KoRuler.h>

class KoRulerController::Private {
public:
    Private(KoRuler *r, KoCanvasResourceProvider *crp) : ruler(r), resourceProvider(crp), lastPosition(-1) {}

    void canvasResourceChanged(int key) {
        if(key != KoText::CurrentTextPosition && key != KoText::CurrentTextDocument)
           return;

        QTextBlock block = currentBlock();
        if(! block.isValid()) {
            ruler->setShowIndents(false);
            ruler->setShowTabs(false);
            return;
        }
        if(block.position() <= lastPosition && block.position() + block.length() > lastPosition)
            return; // nothing changed.
        lastPosition = block.position();

        QTextBlockFormat format = block.blockFormat();
        ruler->setShowIndents(true);
        ruler->setParagraphIndent(format.leftMargin());
        ruler->setFirstLineIndent(format.textIndent());
        ruler->setEndIndent(format.rightMargin());
        ruler->setRightToLeft(block.layout()->textOption().textDirection() == Qt::RightToLeft);
        ruler->setShowTabs(true);

        QList<KoRuler::Tab> tabs;
        QVariant variant = format.property(KoParagraphStyle::TabPositions);
        if(! variant.isNull()) {
            foreach(QVariant var, qvariant_cast<QList<QVariant> >(variant)) {
                KoText::Tab textTab = var.value<KoText::Tab>();
                KoRuler::Tab tab;
                tab.position = textTab.position;
                tab.type = static_cast<KoRuler::TabType> (textTab.type);
                tabs.append(tab);
            }
        }
        ruler->updateTabs(tabs);
    }

    void indentsChanged() {
        QTextBlock block = currentBlock();
        if(! block.isValid())
            return;
        QTextCursor cursor(block);
        QTextBlockFormat bf = cursor.blockFormat();
        bf.setLeftMargin(ruler->paragraphIndent());
        bf.setTextIndent(ruler->firstLineIndent());
        bf.setRightMargin(ruler->endIndent());
        cursor.setBlockFormat(bf);
    }

    void tabsChanged(bool final) {
        Q_UNUSED(final); // TODO use this to cache the tab struct I'm altering.
        QTextBlock block = currentBlock();
        if(! block.isValid())
            return;
        QList<QVariant> list;
        // TODO update the tabs from the parag instead of overwriting them, since this now causes massive dataloss.
        foreach(KoRuler::Tab tab, ruler->tabs()) { // TODO sort on position
            QVariant v;
            KoText::Tab textTab;
            textTab.position = tab.position;
            textTab.type = static_cast<KoText::TabType> (tab.type);
            v.setValue(textTab);
            list.append(v);
        }

        QTextCursor cursor(block);
        QTextBlockFormat bf = cursor.blockFormat();
        bf.setProperty(KoParagraphStyle::TabPositions, list);
        cursor.setBlockFormat(bf);
    }

    QTextBlock currentBlock() {
        QVariant docVar = resourceProvider->resource(KoText::CurrentTextDocument);
        if(docVar.isNull())
            return QTextBlock();
        QTextDocument *doc = static_cast<QTextDocument*> (docVar.value<void*>());
        if(doc == 0)
            return QTextBlock();
        return doc->findBlock(resourceProvider->intResource(KoText::CurrentTextPosition));
    }

private:
    KoRuler *ruler;
    KoCanvasResourceProvider *resourceProvider;
    int lastPosition;
};

KoRulerController::KoRulerController(KoRuler *horizontalRuler, KoCanvasResourceProvider *crp)
    : QObject(horizontalRuler),
    d(new Private(horizontalRuler, crp))
{
    connect(crp, SIGNAL(resourceChanged(int, const QVariant &)), this, SLOT(canvasResourceChanged(int)));
    connect(horizontalRuler, SIGNAL(indentsChanged(bool)), this, SLOT(indentsChanged()));
    connect(horizontalRuler, SIGNAL(tabsChanged(bool)), this, SLOT(tabsChanged(bool)));
}

#include <KoRulerController.moc>

