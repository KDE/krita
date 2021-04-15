/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_categories_mapper_test.h"

#include <simpletest.h>
#include "testing_categories_mapper.h"


void checkItemsList(const QList<TestingMapper::DataItem*> &list,
                    const QStringList &ref)
{
    QCOMPARE(list.size(), ref.size());

    for (int i = 0; i < list.size(); i++) {
        QCOMPARE(list[i]->name(), ref[i]);
    }
}

void checkFetching(TestingMapper &mapper,
                   const QStringList &catsRef,
                   const QStringList &entsRef)
{
    Q_ASSERT(entsRef.isEmpty() || catsRef.size() == entsRef.size());

    for (int i = 0; i < catsRef.size(); i++) {
        if (entsRef.isEmpty() || entsRef[i].isNull()) {
            QVERIFY(mapper.fetchCategory(catsRef[i]));
        } else {
            QVERIFY(mapper.fetchEntry(catsRef[i], entsRef[i]));
        }
    }
}

void KisCategoriesMapperTest::testAddRemoveCategories()
{
    TestingMapper mapper;
    QStringList categories;
    categories << "cat1";
    categories << "cat2";
    categories << "cat3";
    categories << "cat4";

    Q_FOREACH (const QString &cat, categories) {
        mapper.addCategory(cat);
        mapper.checkInsertedCorrectly();
    }

    checkItemsList(mapper.testingGetItems(), categories);
    checkFetching(mapper, categories, QStringList());

    Q_FOREACH (const QString &cat, categories) {
        mapper.removeCategory(cat);
        mapper.checkRemovedCorrectly();

        categories.removeOne(cat);
        checkItemsList(mapper.testingGetItems(), categories);
        checkFetching(mapper, categories, QStringList());
    }
}

void fillLists(QStringList &categories, QStringList &entries, QStringList &items)
{
    categories << "cat1";
    categories << "cat1";
    categories << "cat1";
    categories << "cat2";
    categories << "cat2";
    categories << "cat3";
    categories << "cat4";

    entries << "ent1";
    entries << "ent2";
    entries << "ent3";
    entries << "ent4";
    entries << "ent5";
    entries << "ent6";
    entries << "ent7";

    items << "cat1";
    items << "ent1";
    items << "ent2";
    items << "ent3";
    items << "cat2";
    items << "ent4";
    items << "ent5";
    items << "cat3";
    items << "ent6";
    items << "cat4";
    items << "ent7";
}

void initializeEntries(TestingMapper &mapper, QStringList &categories, QStringList &entries, QStringList &items)
{
    fillLists(categories, entries, items);

    for (int i = 0; i < entries.size(); i++) {
        bool hasCategory = mapper.fetchCategory(categories[i]);
        mapper.addEntry(categories[i], entries[i]);
        mapper.checkInsertedCorrectly(1 + !hasCategory);
    }

    checkItemsList(mapper.testingGetItems(), items);
    checkFetching(mapper, categories, entries);
}

void KisCategoriesMapperTest::testAddRemoveEntries()
{
    TestingMapper mapper;
    QStringList categories;
    QStringList entries;
    QStringList items;

    initializeEntries(mapper, categories, entries, items);

    for (int i = 0, size = entries.size(); i < size; i++) {
        QString cat = categories.takeFirst();
        QString ent = entries.takeFirst();

        mapper.removeEntry(cat, ent);
        mapper.checkRemovedCorrectly();

        items.removeOne(ent);
        checkItemsList(mapper.testingGetItems(), items);
        checkFetching(mapper, categories, entries);
    }
}

void removeCategory(const QString &category, QStringList &categories, QStringList &entries, QStringList &items)
{
    QMutableStringListIterator catIt(categories);
    QMutableStringListIterator entIt(entries);

    while(catIt.hasNext() && entIt.hasNext()) {
        QString cat = catIt.next();
        QString ent = entIt.next();

        if (cat == category) {
            catIt.remove();
            entIt.remove();
        }
    }

    bool started = false;
    QMutableStringListIterator itemsIt(items);
    while(itemsIt.hasNext()) {
        QString item = itemsIt.next();

        if (started && item.startsWith("cat")) break;

        if (started || item == category) {
            itemsIt.remove();
            started = true;
        }
    }
}

void KisCategoriesMapperTest::testRemoveNonEmptyCategories()
{
    TestingMapper mapper;
    QStringList categories;
    QStringList entries;
    QStringList items;

    initializeEntries(mapper, categories, entries, items);

    removeCategory("cat2", categories, entries, items);
    mapper.removeCategory("cat2");
    mapper.checkRemovedCorrectly(3);
    checkItemsList(mapper.testingGetItems(), items);
    checkFetching(mapper, categories, entries);

    removeCategory("cat1", categories, entries, items);
    mapper.removeCategory("cat1");
    mapper.checkRemovedCorrectly(4);
    checkItemsList(mapper.testingGetItems(), items);
    checkFetching(mapper, categories, entries);

    removeCategory("cat3", categories, entries, items);
    mapper.removeCategory("cat3");
    mapper.checkRemovedCorrectly(2);
    checkItemsList(mapper.testingGetItems(), items);
    checkFetching(mapper, categories, entries);

    removeCategory("cat4", categories, entries, items);
    mapper.removeCategory("cat4");
    mapper.checkRemovedCorrectly(2);
    checkItemsList(mapper.testingGetItems(), items);
    checkFetching(mapper, categories, entries);
}

void KisCategoriesMapperTest::testChangingItem()
{
    TestingMapper mapper;
    QStringList categories;
    QStringList entries;
    QStringList items;

    initializeEntries(mapper, categories, entries, items);

    TestingMapper::DataItem *item = mapper.fetchEntry("cat1", "ent2");

    item->setEnabled(false);
    mapper.checkRowChangedIndices(QVector<int>() << 2);

    item->setCheckable(false);
    mapper.checkRowChangedIndices(QVector<int>() << 2);

    item->setChecked(false);
    mapper.checkRowChangedIndices(QVector<int>() << 2);

    mapper.fetchEntry("cat1", "ent3")->setEnabled(false);
    mapper.checkRowChangedIndices(QVector<int>() << 3);

    mapper.fetchCategory("cat1")->setExpanded(false);
    mapper.checkRowChangedIndices(QVector<int>() << 0 << 1 << 2 << 3);
}

SIMPLE_TEST_MAIN(KisCategoriesMapperTest)
