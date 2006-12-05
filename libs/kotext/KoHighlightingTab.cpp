/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006 Thomas Zander <zander@kde.org>

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

#include "KoHighlightingTab.h"
#include "KoText.h"

KoHighlightingTab::KoHighlightingTab( QWidget* parent)
    : QWidget ( parent)
{
    widget.setupUi(this);

    widget.underlineStyle->addItems( KoText::underlineTypeList() );
    widget.underlineLineStyle->addItems( KoText::underlineStyleList() );

    connect( widget.underlineStyle, SIGNAL( activated( int ) ), this, SLOT( underlineChanged( int ) ) );
}

void KoHighlightingTab::underlineChanged(int item) {
    widget.underlineLineStyle->setEnabled(item != 0);
    widget.underlineColor->setEnabled(item != 0);
}

void KoHighlightingTab::open(const QTextCharFormat &format) {
    widget.underlineStyle->setCurrentIndex(1);
    switch(format.underlineStyle()) {
        case QTextCharFormat::SingleUnderline:
            widget.underlineLineStyle->setCurrentIndex(0);
            break;
        case QTextCharFormat::DashUnderline:
            widget.underlineLineStyle->setCurrentIndex(1);
            break;
        case QTextCharFormat::DotLine:
            widget.underlineLineStyle->setCurrentIndex(2);
            break;
        case QTextCharFormat::DashDotLine:
            widget.underlineLineStyle->setCurrentIndex(3);
            break;
        case QTextCharFormat::DashDotDotLine:
            widget.underlineLineStyle->setCurrentIndex(4);
            break;
        case QTextCharFormat::WaveUnderline:
            widget.underlineLineStyle->setCurrentIndex(5);
            break;
        default:
            widget.underlineStyle->setCurrentIndex(0);
            break;
    }
    underlineChanged(widget.underlineStyle->currentIndex());
    widget.underlineColor->setColor(format.underlineColor());

    widget.strikethrough->setChecked(format.fontStrikeOut());
}

void KoHighlightingTab::save(QTextCharFormat &format) const {
    switch(widget.underlineStyle->currentIndex()) {
        case 0: format.setUnderlineStyle(QTextCharFormat::NoUnderline); break;
        case 1:
            QTextCharFormat::UnderlineStyle style;
            switch(widget.underlineLineStyle->currentIndex()) {
                case 0: style = QTextCharFormat::SingleUnderline; break;
                case 1: style = QTextCharFormat::DashUnderline; break;
                case 2: style = QTextCharFormat::DotLine; break;
                case 3: style = QTextCharFormat::DashDotLine; break;
                case 4: style = QTextCharFormat::DashDotDotLine; break;
                case 5: style = QTextCharFormat::WaveUnderline; break;
                default:
                    kWarning() << "Unknown items in the underlineLineStyle combobox!\n";
            }
            format.setUnderlineStyle(style);
            format.setUnderlineColor(widget.underlineColor->color());
            break;
        case 2: // unsupported by Qt right now :(  TODO
            format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            format.setUnderlineColor(widget.underlineColor->color());
            break;
        default:
            kWarning() << "Unknown items in the underlineStyle combobox!\n";
    }

    format.setFontStrikeOut(widget.strikethrough->isChecked());
}

#include "KoHighlightingTab.moc"

