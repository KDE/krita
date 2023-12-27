/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisMplTest.h"

#include <simpletest.h>

#include <optional>
#include <QDebug>

#include <KisMpl.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>


void KisMplTest::testFoldOptional()
{
    std::optional<int> a(0x1);
    std::optional<int> b(0x2);
    std::optional<int> c(0x4);
    std::optional<int> d;
    std::optional<int> e;

    QCOMPARE(kismpl::fold_optional(std::plus{}, a, b, c, d, e), std::optional<int>(7));
    QCOMPARE(kismpl::fold_optional(std::plus{}, e, a, b, c, d), std::optional<int>(7));
    QCOMPARE(kismpl::fold_optional(std::plus{}, d, e, a, b, c), std::optional<int>(7));
    QCOMPARE(kismpl::fold_optional(std::plus{}, c, d, e, a, b), std::optional<int>(7));
    QCOMPARE(kismpl::fold_optional(std::plus{}, b, c, d, e, a), std::optional<int>(7));

    QCOMPARE(kismpl::fold_optional(std::plus{}, b, c, d, e), std::optional<int>(6));
    QCOMPARE(kismpl::fold_optional(std::plus{}, c, d, e), std::optional<int>(4));
    QCOMPARE(kismpl::fold_optional(std::plus{}, d, e), std::optional<int>());
    QCOMPARE(kismpl::fold_optional(std::plus{}, e), std::optional<int>());

}

namespace {
struct Struct {
    Struct(int _id) : id(_id) {}

    int id = -1;
    int idFunc() {
        return id;
    }
    int idConstFunc() const {
        return id;
    }

    int overloaded() const {
        return id;
    }

    int overloaded() {
        return id;
    }

};

struct StructExplicit {
    explicit StructExplicit (int _id) : id(_id) {}

    int id = -1;
    int idConstFunc() const {
        return id;
    }
};

struct StructWithShared : KisShared {
    StructWithShared (int _id) : id(_id) {}

    int id = -1;
    int idConstFunc() const {
        return id;
    }
};

}

void KisMplTest::testMemberOperatorsEqualTo()
{
    int v = 1;
    int &vref = v;
    const int &vconstref = v;


    std::vector<Struct> vec({{0},{1},{2},{3}});

    ////////////////////////////////////////
    // compare member variable against value

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, v));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }

    // compare member variable against reference

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, vref));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }

    // compare member variable against const reference

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, vconstref));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }

    ////////////////////////////////////////
    // compare member function against value

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::idConstFunc, v));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }

    // compare member function against reference

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, vref));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }

    // compare member function against const reference

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, vconstref));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }

    ////////////////////////////////////////
    // compare overloaded member function against value

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::overloaded, v));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }

    // compare member function against reference

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::overloaded, vref));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }

    // compare member function against const reference

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::overloaded, vconstref));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }
}

void KisMplTest::testMemberOperatorsEqualToPointer()
{
    std::vector<Struct> vec_base({{0},{1},{2},{3},{4}});
    std::vector<Struct*> vec({&vec_base[0], &vec_base[1], &vec_base[2], &vec_base[3], &vec_base[3]});

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, 1));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }
}

void KisMplTest::testMemberOperatorsEqualToStdSharedPtr()
{
    std::vector<std::shared_ptr<Struct>> vec({std::make_shared<Struct>(0),
                                              std::make_shared<Struct>(1),
                                              std::make_shared<Struct>(2),
                                              std::make_shared<Struct>(3),
                                              std::make_shared<Struct>(4)});


    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, 1));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }
}

void KisMplTest::testMemberOperatorsEqualToQSharedPointer()
{
    std::vector<QSharedPointer<Struct>> vec({QSharedPointer<Struct>::create(0),
                                             QSharedPointer<Struct>::create(1),
                                             QSharedPointer<Struct>::create(2),
                                             QSharedPointer<Struct>::create(3),
                                             QSharedPointer<Struct>::create(4)});

    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, 1));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }
}

void KisMplTest::testMemberOperatorsEqualToKisSharedPtr()
{
    std::vector<KisSharedPtr<StructWithShared>> vec({new StructWithShared(0),
                                                     new StructWithShared(1),
                                                     new StructWithShared(2),
                                                     new StructWithShared(3),
                                                     new StructWithShared(4)});
    {
        auto it = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&StructWithShared::id, 1));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 1);
    }
}


void KisMplTest::testMemberOperatorsLess()
{
    {
        std::vector<StructExplicit> vec({StructExplicit(0),StructExplicit(1),StructExplicit(2),StructExplicit(3),StructExplicit(4)});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_less(&StructExplicit::id));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 2);
    }

    {
        std::vector<StructExplicit> vec({StructExplicit(0),StructExplicit(1),StructExplicit(2),StructExplicit(3),StructExplicit(4)});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_less(&StructExplicit::idConstFunc));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 2);
    }


    {
        std::vector<StructExplicit> vec_base({StructExplicit(0),StructExplicit(1),StructExplicit(2),StructExplicit(3),StructExplicit(4)});
        std::vector<StructExplicit*> vec({&vec_base[0], &vec_base[1], &vec_base[2], &vec_base[3], &vec_base[3]});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_less(&StructExplicit::id));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 2);
    }

    {
        std::vector<StructExplicit> vec_base({StructExplicit(0),StructExplicit(1),StructExplicit(2),StructExplicit(3),StructExplicit(4)});
        std::vector<StructExplicit*> vec({&vec_base[0], &vec_base[1], &vec_base[2], &vec_base[3], &vec_base[3]});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_less(&StructExplicit::idConstFunc));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 2);
    }

    {
        std::vector<std::shared_ptr<StructExplicit>> vec({std::make_shared<StructExplicit>(0),
                                                          std::make_shared<StructExplicit>(1),
                                                          std::make_shared<StructExplicit>(2),
                                                          std::make_shared<StructExplicit>(3),
                                                          std::make_shared<StructExplicit>(4)});


        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_less(&StructExplicit::id));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 2);
    }

    {
        std::vector<std::shared_ptr<StructExplicit>> vec({std::make_shared<StructExplicit>(0),
                                                          std::make_shared<StructExplicit>(1),
                                                          std::make_shared<StructExplicit>(2),
                                                          std::make_shared<StructExplicit>(3),
                                                          std::make_shared<StructExplicit>(4)});


        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_less(&StructExplicit::idConstFunc));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 2);
    }
}

void KisMplTest::testMemberOperatorsLessEqual()
{
    {
        std::vector<StructExplicit> vec({StructExplicit(0),StructExplicit(1),StructExplicit(2),StructExplicit(3),StructExplicit(4)});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_less_equal(&StructExplicit::id));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 3);
    }

    {
        std::vector<StructExplicit> vec({StructExplicit(0),StructExplicit(1),StructExplicit(2),StructExplicit(3),StructExplicit(4)});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_less_equal(&StructExplicit::idConstFunc));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 3);
    }
}

void KisMplTest::testMemberOperatorsGreater()
{
    {
        std::vector<StructExplicit> vec({StructExplicit(4),StructExplicit(3),StructExplicit(2),StructExplicit(1),StructExplicit(0)});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_greater(&StructExplicit::id));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 2);
    }

    {
        std::vector<StructExplicit> vec({StructExplicit(4),StructExplicit(3),StructExplicit(2),StructExplicit(1),StructExplicit(0)});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_greater(&StructExplicit::idConstFunc));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 2);
    }
}

void KisMplTest::testMemberOperatorsGreaterEqual()
{
    {
        std::vector<StructExplicit> vec({StructExplicit(4),StructExplicit(3),StructExplicit(2),StructExplicit(1),StructExplicit(0)});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_greater_equal(&StructExplicit::id));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 3);
    }

    {
        std::vector<StructExplicit> vec({StructExplicit(4),StructExplicit(3),StructExplicit(2),StructExplicit(1),StructExplicit(0)});

        auto it = std::lower_bound(vec.begin(), vec.end(), 2, kismpl::mem_greater_equal(&StructExplicit::idConstFunc));
        QVERIFY(it != vec.end());
        QCOMPARE(std::distance(vec.begin(), it), 3);
    }
}


SIMPLE_TEST_MAIN(KisMplTest)
