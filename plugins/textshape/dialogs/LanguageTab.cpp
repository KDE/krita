/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "LanguageTab.h"
#include "KoGlobal.h"

#include <KoCharacterStyle.h>

#include <kcombobox.h>
#include <kiconloader.h>

#include <QSet>
#include <QStringList>

#include <LanguageTab.moc>

LanguageTab::LanguageTab(/*KSpell2::Loader::Ptr loader,*/bool uniqueFormat, QWidget* parent, Qt::WFlags fl)
        : QWidget(parent),
        m_uniqueFormat(uniqueFormat)
{
    widget.setupUi(this);

    Q_UNUSED(fl);

    widget.languageListSearchLine->setListWidget(widget.languageList);

    //TODO use fl
    const QStringList langNames = KoGlobal::listOfLanguages();
    const QStringList langTags = KoGlobal::listOfLanguageTags();
    QSet<QString> spellCheckLanguages;
    
    widget.languageList->addItem(QString("None"));
#if 0 //Port it
    if (loader)
        spellCheckLanguages = QSet<QString>::fromList(loader->languages());
#endif
    QStringList::ConstIterator itName = langNames.begin();
    QStringList::ConstIterator itTag = langTags.begin();
    for (; itName != langNames.end() && itTag != langTags.end(); ++itName, ++itTag) {
        if (spellCheckLanguages.contains(*itTag)) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(*itName);
            item->setIcon(SmallIcon("tools-check-spelling"));

            widget.languageList->addItem(item);
        } else
            widget.languageList->addItem(*itName);
    }
    connect(widget.languageList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SIGNAL(languageChanged()));
}

LanguageTab::~LanguageTab()
{
}

void LanguageTab::save(KoCharacterStyle* style) const
{
    if (!widget.languageList->currentItem() || widget.languageList->currentItem()->text() == "None") //TODO i18n
        style->setLanguage(QString());
    else
        style->setLanguage(KoGlobal::tagOfLanguage(widget.languageList->currentItem()->text()));
}

void LanguageTab::setDisplay(KoCharacterStyle *style)
{
    if (m_uniqueFormat) {
        const QString& name = KoGlobal::languageFromTag(style->language());

        QList<QListWidgetItem*> items = widget.languageList->findItems(name,
                                    Qt::MatchFixedString);
        if (!items.isEmpty()) {
            widget.languageList->setCurrentItem(items.first());
            widget.languageList->scrollToItem(items.first());
        }
    }
}
