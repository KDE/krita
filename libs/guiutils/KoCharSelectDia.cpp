/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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

#include "KoCharSelectDia.h"
#include "KoCharSelectDia.moc"

#include <QLayout>
#include <QGridLayout>

#include <klocale.h>
#include <kcharselect.h>
#include <kdebug.h>
#include <KStandardGuiItem>

class KoCharSelectDia::Private {
public:
    Private() : charSelect(0) {}
    KCharSelect *charSelect;
};

/******************************************************************/
/* class KoCharSelectDia                                           */
/******************************************************************/

KoCharSelectDia::KoCharSelectDia( QWidget *parent, const char *name, const QChar &_chr, const QString &_font, bool _modal)
    : KDialog( parent ),
    d(new Private())
{
    setCaption( i18n("Select Character") );
    setModal( _modal );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    setObjectName( name );

    initDialog(_chr,_font);

    KGuiItem okItem = KStandardGuiItem::ok(); // start from std item to keep the OK icon...
    okItem.setText( i18n("&Insert") );
    okItem.setWhatsThis( i18n("Insert the selected character in the text") );
    setButtonGuiItem( KDialog::Ok, okItem );
}

KoCharSelectDia::KoCharSelectDia( QWidget *parent, const char *name, const QString &_font, const QChar &_chr, bool _modal )
    : KDialog( parent ),
    d(new Private())
{
    setCaption( i18n("Select Character") );
    setModal( _modal );
    setButtons( User1 | Close );
    setDefaultButton( User1 );
    setObjectName( name );

    initDialog(_chr,_font);

    setButtonText( User1, i18n("&Insert") );
    setButtonToolTip( User1, i18n("Insert the selected character in the text") );
    connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));
}

void KoCharSelectDia::initDialog(const QChar &_chr, const QString &_font)
{
    QWidget *page = mainWidget()/*plainPage()*/;

    QGridLayout *grid = new QGridLayout( page );
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    d->charSelect = new KCharSelect( page );
    d->charSelect->setCurrentChar( _chr );
    d->charSelect->setCurrentFont( QFont(_font) );
    connect(d->charSelect, SIGNAL(charSelected()),this, SLOT(slotDoubleClicked()));
    d->charSelect->resize( d->charSelect->sizeHint() );
//     d->charSelect->enableFontCombo( true );
    grid->addWidget( d->charSelect, 0, 0 );

    grid->addItem( new QSpacerItem( d->charSelect->width(), 0 ), 0, 0 );
    grid->addItem( new QSpacerItem( 0, d->charSelect->height() ), 0, 0 );
    grid->setRowStretch( 0, 0 );
    d->charSelect->setFocus();
}

KoCharSelectDia::~KoCharSelectDia()
{
    delete d;
}

void KoCharSelectDia::closeDialog()
{
    KDialog::close();
}

// static
bool KoCharSelectDia::selectChar( QString &_font, QChar &_chr, QWidget* parent, const char* name)
{
    bool res = false;

    KoCharSelectDia *dlg = new KoCharSelectDia( parent, name, _chr, _font);
    dlg->setFocus();
    if ( dlg->exec() == Accepted )
    {
        _font = dlg->font();
        _chr = dlg->chr();
        res = true;
    }

    delete dlg;

    return res;
}

QChar KoCharSelectDia::chr() const
{
    return d->charSelect->currentChar();
}

QString KoCharSelectDia::font() const
{
    return d->charSelect->font().family();
}

void KoCharSelectDia::slotUser1()
{
    emit insertChar(chr(),font());
}

void KoCharSelectDia::slotDoubleClicked()
{
    emit insertChar(chr(),font());
}
