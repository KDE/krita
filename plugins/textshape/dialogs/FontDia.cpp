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
#include "CharacterHighlighting.h"
#include "FontDecorations.h"

#include "CharacterGeneral.h"

#include "FormattingPreview.h"

#include <KoTextEditor.h>

#include <klocalizedstring.h>

#include <QTextBlock>
#include <QTextFragment>
#include <QTextDocument>
#include <QTextCursor>

FontDia::FontDia(KoTextEditor *editor, QWidget *parent)
    : KoDialog(parent)
    , m_editor(editor)
    , m_styleChanged(false)
{
    m_initialFormat = m_editor->charFormat();

    setCaption(i18n("Select Font"));
    setModal(true);
    setButtons(Ok | Cancel | Reset | Apply);
    setDefaultButton(Ok);

    m_characterGeneral = new CharacterGeneral(this);
    m_characterGeneral->hideStyleName(true);
    setMainWidget(m_characterGeneral);

    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(resetClicked()), this, SLOT(slotReset()));
    initTabs();

    // Do this after initTabs so it doesn't cause signals prematurely
    connect(m_characterGeneral, SIGNAL(styleChanged()), this, SLOT(styleChanged()));
}

void FontDia::initTabs()
{
    KoCharacterStyle style(m_initialFormat);
    m_characterGeneral->setStyle(&style);
}

void FontDia::styleChanged(bool state)
{
    m_styleChanged = state;
}

void FontDia::slotApply()
{
    if (!m_styleChanged) {
        return;
    }

    m_editor->beginEditBlock(kundo2_i18n("Font"));
    KoCharacterStyle chosenStyle;
    m_characterGeneral->save(&chosenStyle);
    QTextCharFormat cformat;
    chosenStyle.applyStyle(cformat);
    m_editor->mergeAutoStyle(cformat);
    m_editor->endEditBlock();

    m_styleChanged = false;
}

void FontDia::slotOk()
{
    slotApply();
    KoDialog::accept();
}

void FontDia::slotReset()
{
    initTabs();
    slotApply(); // ### Should reset() apply?
}
