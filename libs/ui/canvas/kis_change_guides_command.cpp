/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_change_guides_command.h"

#include "kis_guides_config.h"
#include "KisDocument.h"
#include <kis_image.h>


struct KisChangeGuidesCommand::Private
{
    Private(KisDocument *_doc) : doc(_doc) {}

    KisDocument *doc;

    KisGuidesConfig oldGuides;
    KisGuidesConfig newGuides;
};


KisChangeGuidesCommand::KisChangeGuidesCommand(KisDocument *doc, const KisGuidesConfig &newGuides)
    : KUndo2Command(kundo2_i18n("Edit Guides")),
      m_d(new Private(doc))
{
    m_d->oldGuides = doc->guidesConfig();
    m_d->newGuides = newGuides;
}

KisChangeGuidesCommand::~KisChangeGuidesCommand()
{
}

void KisChangeGuidesCommand::undo()
{
    m_d->doc->setGuidesConfig(m_d->oldGuides);
}

void KisChangeGuidesCommand::redo()
{
    m_d->doc->setGuidesConfig(m_d->newGuides);
}

int KisChangeGuidesCommand::id() const
{
    return 1863;
}

bool KisChangeGuidesCommand::mergeWith(const KUndo2Command *command)
{
    bool result = false;

    const KisChangeGuidesCommand *rhs =
        dynamic_cast<const KisChangeGuidesCommand*>(command);

    if (rhs) {
        m_d->newGuides = rhs->m_d->newGuides;
        result = true;
    }

    return result;
}
