#include "TestChangesDatabase.h"

#include <TextChanges.h>
#include <TextChange.h>

void TestChangesDatabase::testInsert()
{
    TextChanges changes;
    changes.inserted(100, "foobar");
    const TextChange *change = changes.first();
    QVERIFY(change);
    QVERIFY(change->next() == 0);
    QCOMPARE(change->before(), QString());
    QCOMPARE(change->after(), QString("foobar"));
    QCOMPARE(change->position(), 100);
    QCOMPARE(change->length(), 6);

    changes.inserted(200, "lfjsdlkfjsldk");
    QVERIFY(change->previous() == 0);
    QVERIFY(change->next() != 0);
    QCOMPARE(change->position(), 100);
    const TextChange *change2 = change->next();
    QVERIFY(change2->previous() == change);
    QVERIFY(change2->next() == 0);
    QVERIFY(change->previous() == 0);
    QVERIFY(change->next() == change2);
    QCOMPARE(change2->position(), 200);
    QCOMPARE(change->after(), QString("foobar"));
    QCOMPARE(change2->after(), QString("lfjsdlkfjsldk"));

    changes.inserted(20, "sldkfnvlqwepoiz");
    const TextChange *change3 = changes.first();
    QCOMPARE(change3, change->previous());
    QCOMPARE(change3->next(), change);
    QCOMPARE(change3->position(), 20);
    QCOMPARE(change->position(), 115);
    QCOMPARE(change2->position(), 215);
    QCOMPARE(change3->formerPosition(), 20);
    QCOMPARE(change->formerPosition(), 100);
    QCOMPARE(change2->formerPosition(), 200);
}

void TestChangesDatabase::testRemove()
{
    TextChanges changes;
    changes.changed(100, "foobar", "");
    const TextChange *change = changes.first();
    QVERIFY(change);
    QVERIFY(change->next() == 0);
    QCOMPARE(change->before(), QString("foobar"));
    QCOMPARE(change->after(), QString());
    QCOMPARE(change->position(), 100);
    QCOMPARE(change->length(), -6);

    changes.changed(200, "sldkjfnvs", "");
    QVERIFY(change->next());
    const TextChange *change2 = change->next();
    QCOMPARE(change2->previous(), change);
    QCOMPARE(change->next(), change2);
    QCOMPARE(change->position(), 100);
    QVERIFY(change2->next() == 0);
    QVERIFY(change->previous() == 0);
    QCOMPARE(change2->position(), 200);

    changes.changed(20, "lskejf", "");
    const TextChange *change3 = changes.first();
    QCOMPARE(change3->next(), change);
    QCOMPARE(change->previous(), change3);
    QCOMPARE(change3->position(), 20);
    QCOMPARE(change->position(), 94);
    QCOMPARE(change2->position(), 194);
    QCOMPARE(change3->formerPosition(), 20);
    QCOMPARE(change->formerPosition(), 100);
    QCOMPARE(change2->formerPosition(), 200);
}

void TestChangesDatabase::testSplit()
{
    TextChanges changes;
    // test the insertion of a big change and then the revert of a part of that change.
}

void TestChangesDatabase::testMerge()
{
    TextChanges changes;
    changes.inserted(1, "a");

    const TextChange *change = changes.first();
    QVERIFY(change);
    QVERIFY(change->previous() == 0);
    QVERIFY(change->next() == 0);
    QCOMPARE(change->length(), 1);
    QCOMPARE(change->after(), QString("a"));

    changes.inserted(2, "b");
    QVERIFY(change->next() == 0);
    QCOMPARE(change->length(), 2);
    QCOMPARE(change->after(), QString("ab"));
    changes.inserted(3, "c");
    QVERIFY(change->next() == 0);
    QCOMPARE(change->length(), 3);
    QCOMPARE(change->after(), QString("abc"));
    changes.inserted(4, "d");
    QVERIFY(change->next() == 0);
    QCOMPARE(change->length(), 4);
    QCOMPARE(change->after(), QString("abcd"));
    changes.inserted(2, "x");
    QVERIFY(change->next() == 0);
    QCOMPARE(change->length(), 5);
    QCOMPARE(change->after(), QString("axbcd"));

    changes.changed(2, "x", "ka");
    QVERIFY(change->next() == 0);
    QEXPECT_FAIL("", "rest of functionality not implemented yet", Abort);
    QCOMPARE(change->length(), 6);
    QCOMPARE(change->after(), QString("akabcd"));

    // TODO test the insertion of 2 changes and then the insertion of a 3th that makes it 1 change total

    changes.inserted(8, "klm");
    // means we now have;  'akabcd..klm'
    QVERIFY(change->next() != 0);
    changes.changed(6, "xykl", "baz");
    // means we now have;  'akabcdbazm'
    QVERIFY(change->next() == 0);
    QCOMPARE(change->after(), QString("akabcdbazm"));
}

QTEST_MAIN(TestChangesDatabase)
