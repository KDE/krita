/*
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoSectionManager.h"

#include <KoSection.h>
#include <KoParagraphStyle.h>
#include "KoSectionEnd.h"
#include <KLocalizedString>
#include <KoTextDocument.h>

#include <QHash>
#include <QString>
#include <qstack.h>
#include <QTextBlock>

#include <kdebug.h>

KoSectionManagerPrivate::KoSectionManagerPrivate(QTextDocument *_doc)
    : doc(_doc)
    , valid(false)
    , sectionCount(0)
    , model(new QStandardItemModel())
{
    Q_ASSERT(_doc);
}

KoSectionManagerPrivate::~KoSectionManagerPrivate()
{
    QHash<QString, KoSection *>::iterator it = sectionNames.begin();
    for (; it != sectionNames.end(); it++) {
        delete it.value(); // KoSectionEnd will be deleted in KoSection
    }
}

KoSectionManager::KoSectionManager(QTextDocument* doc)
    : d_ptr(new KoSectionManagerPrivate(doc))
{
    KoTextDocument(doc).setSectionManager(this);
}

KoSection *KoSectionManager::sectionAtPosition(int pos)
{
    Q_D(KoSectionManager);
    update();

    KoSection *result = 0;
    int smallest = INT_MAX; //smallest in size section will be the deepest
    QHash<QString, KoSection *>::iterator it = d->sectionNames.begin();
    for (; it != d->sectionNames.end(); it++) {
        if (it.value()->bounds().first > pos || it.value()->bounds().second < pos) {
            continue;
        }

        if (it.value()->bounds().second - it.value()->bounds().first < smallest) {
            result = it.value();
            smallest = result->bounds().second - result->bounds().first;
        }
    }

    return result;
}

void KoSectionManager::invalidate()
{
    Q_D(KoSectionManager);
    d->valid = false;
}

bool KoSectionManager::isValidNewName(const QString &name) const
{
    Q_D(const KoSectionManager);
    return (d->sectionNames.find(name) == d->sectionNames.end());
}

QString KoSectionManager::possibleNewName() const
{
    Q_D(const KoSectionManager);

    QString newName;

    int i = d->sectionCount;
    do {
        ++i;
        newName = i18nc("new numbered section name", "New section %1", i);
    } while (!isValidNewName(newName));

    return newName;
}

void KoSectionManager::registerSection(KoSection* section)
{
    Q_D(KoSectionManager);
    d->sectionCount++;
    d->sectionNames[section->name()] = section;
    invalidate();
}

void KoSectionManager::sectionRenamed(const QString &oldName, const QString &name)
{
    Q_D(KoSectionManager);
    QHash<QString, KoSection *>::iterator it = d->sectionNames.find(oldName);
    KoSection *sec = *it;
    d->sectionNames.erase(it);
    d->sectionNames[name] = sec;

    if (sec->modelItem()) {
        sec->modelItem()->setData(name, Qt::DisplayRole);
    }
}

void KoSectionManager::unregisterSection(KoSection* section)
{
    Q_D(KoSectionManager);

    d->sectionCount--;
    d->sectionNames.remove(section->name());
    invalidate();
}

void KoSectionManager::update()
{
    Q_D(KoSectionManager);
    if (d->valid) {
        return;
    }

    QHash<QString, KoSection *>::iterator it = d->sectionNames.begin();
    for (; it != d->sectionNames.end(); it++) {
        it.value()->setBeginPos(-1);
        it.value()->setEndPos(-1);
        it.value()->setLevel(-1);
    }

    QTextBlock block = d->doc->begin();

    QStringList head;
    head << i18n("Section");
    d->model->clear();
    d->model->setHorizontalHeaderLabels(head);
    d->model->setColumnCount(1);

    QStack<QStandardItem *> curChain;
    curChain.push(d->model->invisibleRootItem());

    int curLevel = -1;
    do {
        QTextBlockFormat fmt = block.blockFormat();

        if (fmt.hasProperty(KoParagraphStyle::SectionStartings)) {
            QList<QVariant> starts = fmt.property(KoParagraphStyle::SectionStartings).value< QList<QVariant> >();
            foreach (const QVariant &sv, starts) {
                curLevel++;
                KoSection *sec = static_cast<KoSection *>(sv.value<void *>());
                sec->setBeginPos(block.position());
                sec->setLevel(curLevel);

                QStandardItem *item = new QStandardItem(sec->name());
                item->setData(qVariantFromValue(static_cast<void *>(sec)), Qt::UserRole + 1);
                sec->setModelItem(item);

                curChain.top()->appendRow(item);

                curChain.push(item);
            }
        }

        if (fmt.hasProperty(KoParagraphStyle::SectionEndings)) {
            QList<QVariant> ends = fmt.property(KoParagraphStyle::SectionEndings).value< QList<QVariant> >();
            foreach (const QVariant &sv, ends) {
                curLevel--;
                KoSectionEnd *sec = static_cast<KoSectionEnd *>(sv.value<void *>());
                sec->correspondingSection()->setEndPos(block.position() + block.length());

                curChain.pop();
            }
        }
    } while ((block = block.next()).isValid());

    d->valid = true;
}

QStandardItemModel* KoSectionManager::sectionsModel()
{
    Q_D(KoSectionManager);
    update();
    return d->model.data();
}
