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

#include "KoLanguageTab.h"
#include "KoGlobal.h"

#include <kcombobox.h>
#include <kiconloader.h>

#include <QSet>
#include <QStringList>

#include "KoLanguageTab.moc"


KoLanguageTab::KoLanguageTab( KSpell2::Loader::Ptr loader, QWidget* parent, const char* name, Qt::WFlags fl ) 
        : KoLanguageTabBase( parent )
{
    Q_UNUSED( name );
    Q_UNUSED( fl );

    languageListSearchLine->setListWidget(languageList);

    //TODO use fl
    const QStringList langNames = KoGlobal::listOfLanguages();
    const QStringList langTags = KoGlobal::listTagOfLanguages();
    QSet<QString> spellCheckLanguages;

    if ( loader )
        spellCheckLanguages = QSet<QString>::fromList(loader->languages());

    QStringList::ConstIterator itName = langNames.begin();
    QStringList::ConstIterator itTag = langTags.begin();
    for ( ; itName != langNames.end() && itTag != langTags.end(); ++itName, ++itTag )
    {
        if ( spellCheckLanguages.contains( *itTag ) )
        {
            QListWidgetItem* item = new QListWidgetItem();
            item->setText( *itName );
            item->setIcon( SmallIcon("tools-check-spelling") );

            languageList->addItem(item); 
        }
        else
            languageList->addItem( *itName );
    }
    connect( languageList, SIGNAL( currentItemChanged( QListWidgetItem*,QListWidgetItem* ) ), 
            this, SIGNAL( languageChanged() ) );
}

KoLanguageTab::~KoLanguageTab()
{
}

QString KoLanguageTab::getLanguage() const
{
    Q_ASSERT( languageList->currentItem() );

    return KoGlobal::tagOfLanguage( languageList->currentItem()->text() );
}

void KoLanguageTab::setLanguage( const QString &item )
{
    const QString& name = KoGlobal::languageFromTag(item);

    QList<QListWidgetItem*> items = languageList->findItems(name,
                                                            Qt::MatchFixedString);
    if ( !items.isEmpty() )
    {
        languageList->setCurrentItem(items.first());
        languageList->scrollToItem(items.first());
    }
}
