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

#include <QList>
#include <QListIterator>

struct KisChangeGuidesCommand::Private
{
    Private(KisDocument *_doc) : doc(_doc) {}

    bool sameOrOnlyMovedOneGuideBetween(const KisGuidesConfig &first, const KisGuidesConfig &second);
    enum Status {
        NO_DIFF = 0,
        ONE_DIFF = 1, /// only one difference including adding or removing
        OTHER_DIFF = 4
    };
    Status diff(const QList<qreal> &first, const QList<qreal> &second);

    KisDocument *doc;

    KisGuidesConfig oldGuides;
    KisGuidesConfig newGuides;
};

bool KisChangeGuidesCommand::Private::sameOrOnlyMovedOneGuideBetween(const KisGuidesConfig &first, const KisGuidesConfig &second)
{
    return diff(first.horizontalGuideLines(), second.horizontalGuideLines()) +
        diff(first.verticalGuideLines(), second.verticalGuideLines()) <= 1;
}

KisChangeGuidesCommand::Private::Status KisChangeGuidesCommand::Private::diff(const QList<qreal> &first, const QList<qreal> &second)
{
    if (first.size() == second.size()) {
        int diffCount = 0;
        for (int i = 0; i < first.size(); ++i) {
            if (first[i] != second[i]) {
                ++diffCount;
                if (diffCount > 1) {
                    return OTHER_DIFF;
                }
            }
        }
        return diffCount == 0 ? NO_DIFF : ONE_DIFF;
    } else if (first.size() - second.size() == -1) { // added a guide
        QList<qreal> beforeRemoval = second;
        beforeRemoval.takeLast();
        return first == beforeRemoval ? ONE_DIFF : OTHER_DIFF;
    } else if (first.size() - second.size() == 1) { // removed a guide
        bool skippedItem = false;
        for (QListIterator<qreal> i(first), j(second); i.hasNext() && j.hasNext(); ) {
            qreal curFirst = i.next();
            qreal curSecond = j.next();
            if (!skippedItem && curFirst != curSecond) {
                curFirst = i.next(); // try to go to the next item and see if it matches
            }
            if (curFirst != curSecond) {
                return OTHER_DIFF;
            }
        }
        // here we conclude only one guide is removed
        return ONE_DIFF;
    } else {
        return OTHER_DIFF;
    }
}

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
        // we want to only merge consecutive movements, or creation then movement, or movement then deletion
        // there should not be any changes not on the stack (see kis_guides_manager.cpp)
        // nor any addition/removal of guides
        // nor the movement of other guides
        if (m_d->newGuides == rhs->m_d->oldGuides &&
            m_d->sameOrOnlyMovedOneGuideBetween(m_d->oldGuides, rhs->m_d->newGuides)) {
            m_d->newGuides = rhs->m_d->newGuides;
            result = true;
        }
    }

    return result;
}
