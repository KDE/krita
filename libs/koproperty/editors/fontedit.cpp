/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2005-2009 Jarosław Staniek <staniek@kde.org>

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

#include "fontedit.h"
#include "utils.h"

#include <KFontChooser>
#include <KFontDialog>
#include <KLocale>
#include <KDebug>
#include <KPushButton>
#include <KDialog>

#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QModelIndex>
#include <QVariant>
#include <QStyleOptionViewItem>
#include <QFontDatabase>
#include <QEvent>
#include <QHBoxLayout>
#include <QApplication>

using namespace KoProperty;

//! @internal
//! reimplemented to better button and label's positioning
class FontEditRequester : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QFont value READ value WRITE setValue USER true)
public:
    FontEditRequester(QWidget* parent)
            : QWidget(parent)
            , m_paletteChangedEnabled(true)
    {
        setBackgroundRole(QPalette::Base);
        QHBoxLayout *lyr = new QHBoxLayout(this);
        lyr->setContentsMargins(0,0,0,0);
        lyr->setSpacing( 1 );
        lyr->addStretch(1);
        m_button = new KPushButton(this);
        setFocusProxy(m_button);
        Utils::setupDotDotDotButton(m_button,
            i18n("Click to select a font"),
            i18n("Selects font"));
        connect( m_button, SIGNAL( clicked() ), SLOT( slotSelectFontClicked() ) );
        lyr->addWidget(m_button);
        setValue(qApp->font());
    }

    QFont value() const
    {
        return m_font;
    }

public slots:
    void setValue(const QFont& value)
    {
        //kDebug() << QFontDatabase().families();
        m_font = value;
    }

signals:
    void commitData( QWidget * editor );

protected slots:
    void slotSelectFontClicked()
    {
        KFontChooser::DisplayFlags flags = KFontChooser::NoDisplayFlags;
        if (KDialog::Accepted == KFontDialog::getFont( m_font, flags, this )) {
            setValue(m_font);
            emit commitData(this);
        }
    }

protected:
    virtual bool event( QEvent * event )
    {
        return QWidget::event(event);
    }

    KPushButton *m_button;
    QFont m_font;
    bool m_paletteChangedEnabled;
};

// -----------

QWidget * FontDelegate::createEditor( int type, QWidget *parent, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    return new FontEditRequester(parent);
}

void FontDelegate::paint( QPainter * painter, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    painter->save();
    const QFont origFont( painter->font() );
    QFont f( index.data(Qt::EditRole).value<QFont>() );
    int size = f.pointSize(); // will be needed later
    if (size == -1) {
        size = f.pixelSize();
    }
    if (option.font.pointSize() > 0)
        f.setPointSize(option.font.pointSize());
    else if (option.font.pixelSize() > 0)
        f.setPixelSize(option.font.pixelSize());
    painter->setFont( f );
    QRect rect( option.rect );
    rect.setLeft( rect.left() + 1 );
    const QString txt( i18nc("Font sample for property editor item, typically \"Abc\"", "Abc") );
    painter->drawText( rect, Qt::AlignLeft | Qt::AlignVCenter, 
        i18nc("Font sample for property editor item, typically \"Abc\"", "Abc") );

    rect.setLeft(rect.left() + 5 + painter->fontMetrics().width( txt ));
    painter->setFont(origFont);
    painter->drawText( rect, Qt::AlignLeft | Qt::AlignVCenter, 
        i18nc("Font family and size, e.g. Arial, 2pt", "%1, %2pt", f.family(), size) );
    painter->restore();
}

#include "fontedit.moc"
