/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_change_guides_command.h"

#include "kis_guides_config.h"
#include "KisDocument.h"
#include <kis_image.h>

#include <QList>
#include <QListIterator>

struct KisChangeGuidesCommand::Private
{
    Private(KisDocument *_doc, KisChangeGuidesCommand *q) : doc(_doc), q(q), firstRedo(true) {}

    bool sameOrOnlyMovedOneGuideBetween(const KisGuidesConfig &first, const KisGuidesConfig &second);
    enum Status {
        NO_DIFF = 0,
        ONE_DIFF = 1,
        ADDITION = 4,
        REMOVAL = 16,
        OTHER_DIFF = 1024
    };
    Status diff(const QList<qreal> &first, const QList<qreal> &second);

    void switchTo(const KisGuidesConfig &config);

    KisDocument *doc;
    KisChangeGuidesCommand *q;

    KisGuidesConfig oldGuides;
    KisGuidesConfig newGuides;

    bool firstRedo;
};

bool KisChangeGuidesCommand::Private::sameOrOnlyMovedOneGuideBetween(const KisGuidesConfig &first, const KisGuidesConfig &second)
{
    int ret = diff(first.horizontalGuideLines(), second.horizontalGuideLines()) +
        diff(first.verticalGuideLines(), second.verticalGuideLines());

    if (ret == ADDITION) {
        q->setText(kundo2_i18n("Add Guide"));
    } else if (ret == REMOVAL) {
        q->setText(kundo2_i18n("Remove Guide"));
    } else if (ret == NO_DIFF || ret == ONE_DIFF) { // meaning we will still merge it
        // XXX: how to deal with NO_DIFF (the command "should" be removed -- how?)
        q->setText(kundo2_i18n("Edit Guides"));
    } else {
        return false;
    }
    return true;
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
        return first == beforeRemoval ? ADDITION : OTHER_DIFF;
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
        return REMOVAL;
    } else {
        return OTHER_DIFF;
    }
}

void KisChangeGuidesCommand::Private::switchTo(const KisGuidesConfig &config)
{
    KisGuidesConfig curConfig = doc->guidesConfig();
    curConfig.setHorizontalGuideLines(config.horizontalGuideLines());
    curConfig.setVerticalGuideLines(config.verticalGuideLines());
    doc->setGuidesConfig(curConfig);
}

KisChangeGuidesCommand::KisChangeGuidesCommand(KisDocument *doc, const KisGuidesConfig &oldGuides, const KisGuidesConfig &newGuides)
    : KUndo2Command(kundo2_i18n("Edit Guides")),
      m_d(new Private(doc, this))
{
    m_d->oldGuides = oldGuides;
    m_d->newGuides = newGuides;
    // update the undo command text
    m_d->sameOrOnlyMovedOneGuideBetween(m_d->oldGuides, m_d->newGuides);
}

KisChangeGuidesCommand::~KisChangeGuidesCommand()
{
}

void KisChangeGuidesCommand::undo()
{
    m_d->switchTo(m_d->oldGuides);
}

void KisChangeGuidesCommand::redo()
{
    if (m_d->firstRedo) {
        m_d->firstRedo = false;
        return;
    }
    m_d->switchTo(m_d->newGuides);
}

int KisChangeGuidesCommand::id() const
{
    return 1863;
}

