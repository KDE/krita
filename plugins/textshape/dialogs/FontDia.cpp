/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C)  2008 Girish Ramakrishnan <girish@forwardbias.in>
   Copyright (C)  2008 Pierre Stirnweiss <pstirnweiss@googlemail.com>

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

#include "FontDia.h"
#include "FontTab.h"
#include "CharacterHighlighting.h"
#include "FontDecorations.h"
#include "FontLayoutTab.h"

#include "CharacterGeneral.h"

#include "FormattingPreview.h"

#include <klocale.h>
#include <kvbox.h>
#include <kfontdialog.h>

#include <QTextBlock>
#include <QTextFragment>
#include <QTextDocument>
#include <QTextCursor>

FontDia::FontDia(QTextCursor* cursor, QWidget* parent)
        : KDialog(parent),
        m_cursor(cursor)
{
    //First find out if we have more than one charFormat in our selection. If so, m_initialFormat/m_style will get initialised with the charFormat at the cursor's position. The tabs will get informed of this.

    if (m_cursor->hasSelection()) {
        int begin = qMin(m_cursor->anchor(), m_cursor->position());
        int end = qMax(m_cursor->anchor(), m_cursor->position());
        QTextBlock block = m_cursor->block().document()->findBlock(begin);
        m_uniqueFormat = true;
        QTextCursor caret(*m_cursor);
        caret.setPosition(begin+1);
        m_initialFormat = caret.charFormat();
        while (block.isValid() && block.position() < end) {
            QTextBlock::iterator iter = block.begin();
            while (! iter.atEnd()) {
                QTextFragment fragment = iter.fragment();
                if (fragment.position() >= end)
                    break;
                if (fragment.position() + fragment.length() <= begin) {
                    iter++;
                    continue;
                }
                if (!(m_uniqueFormat = (fragment.charFormat() == m_initialFormat)))
                    break;
                iter++;
            }
            if (!m_uniqueFormat)
                break;
            block = block.next();
        }
    }
    else {
        m_initialFormat = cursor->charFormat();
        m_uniqueFormat = true;
    }

    setCaption(i18n("Select Font"));
    setModal(true);
    setButtons(Ok | Cancel | Reset | Apply);
    setDefaultButton(Ok);

    m_characterGeneral = new CharacterGeneral(this, m_uniqueFormat);
    m_characterGeneral->hideStyleName(true);
    setMainWidget(m_characterGeneral);

    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(resetClicked()), this, SLOT(slotReset()));
    initTabs();
}

void FontDia::initTabs()
{
    KoCharacterStyle style(m_initialFormat);
    m_characterGeneral->setStyle(&style);
}

void FontDia::slotApply()
{
    emit startMacro(i18n("Font"));
    KoCharacterStyle chosenStyle;
    m_characterGeneral->save(&chosenStyle);
    chosenStyle.applyStyle(m_cursor);
    emit stopMacro();
}

void FontDia::slotOk()
{
    slotApply();
    KDialog::accept();
}

void FontDia::slotReset()
{
    initTabs();
    slotApply(); // ### Should reset() apply?
}

#include <FontDia.moc>
