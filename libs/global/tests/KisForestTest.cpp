/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisForestTest.h"

#include "KisCppQuirks.h"
#include "KisForest.h"
#include <vector>

struct IteratorToValue
{
    using value_type = int;

    template <typename Iterator>
    value_type operator() (Iterator it) const {
        return *it;
    }
};

template <typename Iterator, typename IteratorValuePolicy = IteratorToValue>
bool testForestIteration(Iterator begin, Iterator end,
                         std::vector<typename IteratorValuePolicy::value_type> reference,
                         IteratorValuePolicy iteratorToValue = IteratorValuePolicy())
{
    using value_type = typename IteratorValuePolicy::value_type;

    bool result = true;
    std::vector<value_type> values;

    std::size_t index = 0;
    for (auto it = begin; it != end; ++it, ++index) {
        value_type value = iteratorToValue(it);

        if (index >= reference.size() || value != reference[index]) {
            result = false;
        }
        values.push_back(value);

        // emergency exit in case of infinite loop :)
        // "40 forest items must be enough for everyone!" (c)
        if (index > 40) {
            result = false;
            break;
        }
    }

    result &= values.size() == reference.size();

    if (!result) {
        qDebug() << "Failed forest iteration:";
        {
            QDebug deb = qDebug();
            deb << "    result:";
            Q_FOREACH(value_type value, values) {
                deb << value;
            }
        }
        {
            QDebug deb = qDebug();
            deb << "    ref.  :";
            Q_FOREACH(value_type value, reference) {
                deb << value;
            }
        }
    }

    return result;
}

void KisForestTest::testAddToRoot()
{
    KisForest<int> forest;

    forest.insert(childBegin(forest), 2);
    forest.insert(childBegin(forest), 1);
    forest.insert(childBegin(forest), 0);
    forest.insert(childEnd(forest), 3);

    QVERIFY(testForestIteration(childBegin(forest), childEnd(forest), {0,1,2,3}));

}

void KisForestTest::testAddToRootChained()
{
    KisForest<int> forest;

    auto it = forest.insert(childBegin(forest), 3);
    it = forest.insert(it, 2);
    it = forest.insert(it, 1);
    it = forest.insert(it, 0);

    QVERIFY(testForestIteration(childBegin(forest), childEnd(forest), {0,1,2,3}));
}

void KisForestTest::testAddToLeaf()
{
    KisForest<int> forest;

    auto root = forest.insert(childBegin(forest), 0);
    forest.insert(childBegin(root), 2);
    forest.insert(childBegin(root), 1);
    forest.insert(childEnd(root), 3);
    forest.insert(childEnd(root), 4);

    QVERIFY(testForestIteration(childBegin(forest), childEnd(forest), {0}));
    QVERIFY(testForestIteration(childBegin(root), childEnd(root), {1,2,3,4}));

}

void KisForestTest::testAddToLeafChained()
{
    KisForest<int> forest;

    auto root = forest.insert(childBegin(forest), 0);
    auto it = forest.insert(childBegin(root), 4);
    it = forest.insert(it, 3);
    it = forest.insert(it, 2);
    it = forest.insert(it, 1);

    QVERIFY(testForestIteration(childBegin(forest), childEnd(forest), {0}));
    QVERIFY(testForestIteration(childBegin(root), childEnd(root), {1,2,3,4}));

}

void KisForestTest::testDFSIteration()
{
    KisForest<int> forest;

    /**
     * 0 1
     *   2
     *   3 5 6
     *       7
     *   4
     * 8 9
     *   10
     **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);

    QVERIFY(testForestIteration(childBegin(forest), childEnd(forest), {0, 8}));
    QVERIFY(testForestIteration(childBegin(it0), childEnd(it0), {1,2,3,4}));
    QVERIFY(testForestIteration(childBegin(it8), childEnd(it8), {9,10}));
    QVERIFY(testForestIteration(childBegin(it3), childEnd(it3), {5}));
    QVERIFY(testForestIteration(childBegin(it5), childEnd(it5), {6,7}));

    QVERIFY(testForestIteration(begin(forest), end(forest), {0,1,2,3,5,6,7,4,8,9,10}));
}

void KisForestTest::testHierarchyIteration()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);

    QVERIFY(testForestIteration(childBegin(forest), childEnd(forest), {0, 8}));
    QVERIFY(testForestIteration(childBegin(it0), childEnd(it0), {1,2,3,4}));
    QVERIFY(testForestIteration(childBegin(it8), childEnd(it8), {9,10}));
    QVERIFY(testForestIteration(childBegin(it3), childEnd(it3), {5}));
    QVERIFY(testForestIteration(childBegin(it5), childEnd(it5), {6,7}));

    QVERIFY(testForestIteration(hierarchyBegin(it0), hierarchyEnd(it0), {0}));
    QVERIFY(testForestIteration(hierarchyBegin(forest.end()), hierarchyEnd(forest.end()), {}));

    QVERIFY(testForestIteration(hierarchyBegin(it7), hierarchyEnd(it7), {7,5,3,0}));
    QVERIFY(testForestIteration(hierarchyBegin(it5), hierarchyEnd(it5), {5,3,0}));
    QVERIFY(testForestIteration(hierarchyBegin(it4), hierarchyEnd(it4), {4,0}));
    QVERIFY(testForestIteration(hierarchyBegin(it10), hierarchyEnd(it10), {10,8}));
}

void KisForestTest::testSiblingIteration()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);

    /**
     * Test if all types of iterators are convertible into a child iterator
     */
    QVERIFY(testForestIteration(siblingCurrent(it2), siblingEnd(it2), {2,3,4}));
    QVERIFY(testForestIteration(siblingCurrent(hierarchyBegin(it2)), siblingEnd(hierarchyBegin(it2)), {2,3,4}));
    QVERIFY(testForestIteration(siblingCurrent(subtreeBegin(it2)), siblingEnd(subtreeBegin(it2)), {2,3,4}));
    QVERIFY(testForestIteration(siblingCurrent(tailSubtreeBegin(it2)), siblingEnd(tailSubtreeBegin(it2)), {2,3,4}));
    QVERIFY(testForestIteration(siblingCurrent(compositionBegin(it2)), siblingEnd(compositionBegin(it2)), {2,3,4}));

    // we cannot create a child iterator from an non-child end-iterator
    //    QVERIFY(testForestIteration(siblingCurrent(compositionBegin(childEnd(it0))),
    //                                siblingEnd(compositionBegin(childEnd(it0))), {}));
}

void KisForestTest::testCompositionIteration()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);

    QVERIFY(testForestIteration(compositionBegin(forest), compositionEnd(forest), {0, 1, 1, 2, 2, 3, 5, 6, 6, 7, 7, 5, 3, 4, 4, 0, 8, 9, 9, 10, 10, 8}));

}

struct CompositionIteratorPairedValue
{
    using value_type = std::pair<int, KisForest<int>::composition_iterator::traversal_state>;

    template <typename Iterator>
    value_type operator() (Iterator it) const {
        return std::make_pair(*it, it.state());
    }
};

void KisForestTest::testCompositionIterationSubtree()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);

    QVERIFY(testForestIteration(compositionBegin(it3), compositionEnd(it3), {3, 5, 6, 6, 7, 7, 5, 3}));
    QVERIFY(testForestIteration(compositionBegin(it5), compositionEnd(it5), {5, 6, 6, 7, 7, 5}));
    QVERIFY(testForestIteration(compositionBegin(it8), compositionEnd(it8), {8, 9, 9, 10, 10, 8}));

    using traversal_direction = KisForest<int>::composition_iterator::traversal_state;

    std::vector<std::pair<int, traversal_direction>> references;

    references = {{5, traversal_direction::Enter},
                  {6, traversal_direction::Enter},
                  {6, traversal_direction::Leave},
                  {7, traversal_direction::Enter},
                  {7, traversal_direction::Leave},
                  {5, traversal_direction::Leave}};

    QVERIFY(testForestIteration(compositionBegin(it5), compositionEnd(it5),
                              references, CompositionIteratorPairedValue()));

    references = {{3, traversal_direction::Enter},
                  {5, traversal_direction::Enter},
                  {6, traversal_direction::Enter},
                  {6, traversal_direction::Leave},
                  {7, traversal_direction::Enter},
                  {7, traversal_direction::Leave},
                  {5, traversal_direction::Leave},
                  {3, traversal_direction::Leave}};

    QVERIFY(testForestIteration(compositionBegin(it3), compositionEnd(it3),
                              references, CompositionIteratorPairedValue()));

}

void KisForestTest::testSubtreeIteration()
{
    KisForest<int> forest;

    /**
     * 0 1
     *   2
     *   3 5 6
     *       7
     *   4
     * 8 9
     *   10
     **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);


    QVERIFY(testForestIteration(subtreeBegin(it3), subtreeEnd(it3), {3,5,6,7}));
    QVERIFY(testForestIteration(subtreeBegin(it0), subtreeEnd(it0), {0,1,2,3,5,6,7,4}));
    QVERIFY(testForestIteration(subtreeBegin(it8), subtreeEnd(it8), {8,9,10}));
}

void KisForestTest::testSubtreeTailIteration()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);


    QVERIFY(testForestIteration(tailSubtreeBegin(it3), tailSubtreeEnd(it3), {6,7,5,3}));
    QVERIFY(testForestIteration(tailSubtreeBegin(it0), tailSubtreeEnd(it0), {1,2,6,7,5,3,4,0}));
    QVERIFY(testForestIteration(tailSubtreeBegin(it8), tailSubtreeEnd(it8), {9,10,8}));

    QVERIFY(testForestIteration(tailSubtreeBegin(forest), tailSubtreeEnd(forest), {1,2,6,7,5,3,4,0,9,10,8}));
}

void KisForestTest::testEraseNode()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);


    QCOMPARE(forest.erase(it6), it7);
    QVERIFY(testForestIteration(begin(forest), end(forest), {0,1,2,3,5,7,4,8,9,10}));

    QCOMPARE(forest.erase(it7), childEnd(it5));
    QVERIFY(testForestIteration(begin(forest), end(forest), {0,1,2,3,5,4,8,9,10}));

    QCOMPARE(forest.erase(it10), childEnd(it8));
    QVERIFY(testForestIteration(begin(forest), end(forest), {0,1,2,3,5,4,8,9}));

    QCOMPARE(forest.erase(it9), childEnd(it8));
    QVERIFY(testForestIteration(begin(forest), end(forest), {0,1,2,3,5,4,8}));

    QCOMPARE(forest.erase(it8), childEnd(forest));
    QVERIFY(testForestIteration(begin(forest), end(forest), {0,1,2,3,5,4}));
}

void KisForestTest::testEraseSubtree()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);


    QCOMPARE(forest.erase(it3), it4);
    QVERIFY(testForestIteration(begin(forest), end(forest), {0,1,2,4,8,9,10}));

    QCOMPARE(forest.erase(it8), childEnd(forest));
    QVERIFY(testForestIteration(begin(forest), end(forest), {0,1,2,4}));

    QCOMPARE(forest.erase(it0), childEnd(forest));
    QVERIFY(testForestIteration(begin(forest), end(forest), {}));
}

void KisForestTest::testEraseRange()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         */


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);


    QCOMPARE(forest.erase(it1, it4), it4);
    QVERIFY(testForestIteration(begin(forest), end(forest), {0,4,8,9,10}));

    QCOMPARE(forest.erase(it4, childEnd(it0)), childEnd(it0));
    QVERIFY(testForestIteration(begin(forest), end(forest), {0,8,9,10}));
}

void KisForestTest::testMoveSubtree()
{
    KisForest<int> forest;

    /**
     * 0 1
     *   2
     *   3 5 6
     *       7
     *   4
     * 8 9
     *   10
     */


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);


    auto newPos = forest.move(it3, it9);
    QCOMPARE(newPos, childBegin(it8));

    QVERIFY(testForestIteration(begin(forest), end(forest), {0,1,2,4,8,3,5,6,7,9,10}));

    newPos = forest.move(it0, childEnd(it8));
    QCOMPARE(newPos, std::prev(childEnd(it8)));

    QVERIFY(testForestIteration(begin(forest), end(forest), {8,3,5,6,7,9,10,0,1,2,4}));
}

void KisForestTest::testReversedChildIteration()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);

    QVERIFY(testForestIteration(make_reverse_iterator(childEnd(forest)),
                                make_reverse_iterator(childBegin(forest)), {8, 0}));

    QVERIFY(testForestIteration(make_reverse_iterator(childEnd(it0)),
                                make_reverse_iterator(childBegin(it0)), {4,3,2,1}));

    QVERIFY(testForestIteration(make_reverse_iterator(childEnd(it3)),
                                make_reverse_iterator(childBegin(it3)), {5}));
}

void KisForestTest::testConversionsFromEnd()
{
    KisForest<int> forest;

    /**
         * 0 1
         *   2
         *   3 5 6
         *       7
         *   4
         * 8 9
         *   10
         **/


    auto it0 = forest.insert(childBegin(forest), 0);
    auto it8 = forest.insert(childEnd(forest), 8);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);

    /**
     * Currently, all operations on end-iterators are declared as "undefined behavior",
     * but, ideally, we should care about them. Like, it should be possible to get children
     * of the forest by calling childBegin/End(hierarchyEnd(it0)). I (DK) am not sure if
     * it is possible to implement without overhead. So I just added this test to document
     * "desired" behavior. But for now, no one should rely on this behavior, just consider
     * all operations with end-iterators as UB.
     */

#define HIDE_UB_NOISE

    QVERIFY(testForestIteration(childBegin(childEnd(it0)),
                              childEnd(childEnd(it0)), {}));

#ifndef HIDE_UB_NOISE
    QEXPECT_FAIL("", "Fetching children of root node should return roots of the forest?", Continue);
    QVERIFY(testForestIteration(childBegin(hierarchyEnd(it0)),
                              childEnd(hierarchyEnd(it0)), {0, 8}));
    QEXPECT_FAIL("", "End of composition should not (?) point to any existing node", Continue);
    QVERIFY(testForestIteration(childBegin(compositionEnd(it0)),
                              childEnd(compositionEnd(it0)), {}));
#endif

    QVERIFY(testForestIteration(hierarchyBegin(childEnd(it0)),
                              hierarchyEnd(childEnd(it0)), {}));
    QVERIFY(testForestIteration(hierarchyBegin(hierarchyEnd(it0)),
                              hierarchyEnd(hierarchyEnd(it0)), {}));
#ifndef HIDE_UB_NOISE
    QEXPECT_FAIL("", "End of composition should not (?) point to any existing node", Continue);
    QVERIFY(testForestIteration(hierarchyBegin(compositionEnd(it0)),
                              hierarchyEnd(compositionEnd(it0)), {}));
#endif

    QVERIFY(testForestIteration(compositionBegin(childEnd(it0)),
                              compositionEnd(childEnd(it0)), {}));
#ifndef HIDE_UB_NOISE
    QEXPECT_FAIL("", "Starting composition on the root node should behave like we do it for the forest itself?", Continue);
    QVERIFY(testForestIteration(compositionBegin(hierarchyEnd(it0)),
                              compositionEnd(hierarchyEnd(it0)), {0, 1, 1, 2, 2, 3, 5, 6, 6, 7, 7, 5, 3, 4, 4, 0, 8, 9, 9, 10, 10, 8}));
    QEXPECT_FAIL("", "End of composition should not (?) point to any existing node", Continue);
    QVERIFY(testForestIteration(compositionBegin(compositionEnd(it0)),
                              compositionEnd(compositionEnd(it0)), {}));
#endif

#ifndef HIDE_UB_NOISE
    QEXPECT_FAIL("", "Fetching siblings from child-end should work?", Continue);
    QVERIFY(testForestIteration(siblingBegin(childEnd(it0)),
                              siblingEnd(childEnd(it0)), {1,2,3,4}));
#endif

    // TODO: we currently don't allow creation of child-iterators from end-non-child-iterators
//    QVERIFY(testForestIteration(siblingBegin(hierarchyEnd(it0)),
//                                siblingEnd(hierarchyEnd(it0)), {}));
//    QVERIFY(testForestIteration(siblingBegin(compositionEnd(it0)),
//                                siblingEnd(compositionEnd(it0)), {0,8}));



    QVERIFY(testForestIteration(childBegin(childEnd(forest)),
                              childEnd(childEnd(forest)), {}));
    // TODO: we currently don't allow creation of child-iterators from end-non-child-iterators
//    QVERIFY(testForestIteration(childBegin(compositionEnd(forest)),
//                                childEnd(compositionEnd(forest)), {}));

    QVERIFY(testForestIteration(hierarchyBegin(childEnd(forest)),
                              hierarchyEnd(childEnd(forest)), {}));
    QVERIFY(testForestIteration(hierarchyBegin(compositionEnd(forest)),
                              hierarchyEnd(compositionEnd(forest)), {}));

    QVERIFY(testForestIteration(compositionBegin(childEnd(forest)),
                              compositionEnd(childEnd(forest)), {}));
    QVERIFY(testForestIteration(compositionBegin(compositionEnd(forest)),
                              compositionEnd(compositionEnd(forest)), {}));

#ifndef HIDE_UB_NOISE
    QEXPECT_FAIL("", "Fetching siblings from forest's child-end should work?", Continue);
    QVERIFY(testForestIteration(siblingBegin(childEnd(forest)),
                              siblingEnd(childEnd(forest)), {0,8}));
    QEXPECT_FAIL("", "Fetching siblings from forest's composition-end should work?", Continue);
    QVERIFY(testForestIteration(siblingBegin(compositionEnd(forest)),
                              siblingEnd(compositionEnd(forest)), {0,8}));
#endif

#undef HIDE_UB_NOISE
}

void KisForestTest::testCopyForest()
{
    KisForest<int> forest;

    /**
     * 0 1
     *   2
     *   3 5 6
     *       7
     *   4
     *   8 9
     *     10
     **/


    auto it0 = forest.insert(childBegin(forest), 0);

    auto it1 = forest.insert(childEnd(it0), 1);
    auto it2 = forest.insert(childEnd(it0), 2);
    auto it3 = forest.insert(childEnd(it0), 3);
    auto it4 = forest.insert(childEnd(it0), 4);
    auto it8 = forest.insert(childEnd(it0), 8);

    auto it5 = forest.insert(childEnd(it3), 5);

    auto it6 = forest.insert(childEnd(it5), 6);
    auto it7 = forest.insert(childEnd(it5), 7);

    auto it9 = forest.insert(childEnd(it8), 9);
    auto it10 = forest.insert(childEnd(it8), 10);

    Q_UNUSED(it1);
    Q_UNUSED(it2);
    Q_UNUSED(it6);
    Q_UNUSED(it7);
    Q_UNUSED(it4);
    Q_UNUSED(it9);
    Q_UNUSED(it10);

    QVERIFY(testForestIteration(begin(forest),
                                end(forest),
                                {0, 1, 2, 3, 5, 6, 7, 4, 8, 9, 10}));


    {
        KisForest<int> clonedForest = forest;

        QVERIFY(testForestIteration(begin(clonedForest),
                                    end(clonedForest),
                                    {0, 1, 2, 3, 5, 6, 7, 4, 8, 9, 10}));
    }

    {
        const KisForest<int> &constForest = forest;
        KisForest<int> clonedForest = constForest;

        QVERIFY(testForestIteration(begin(clonedForest),
                                    end(clonedForest),
                                    {0, 1, 2, 3, 5, 6, 7, 4, 8, 9, 10}));
    }

}

void KisForestTest::testSwapForest()
{
    /**
     * 0 1
     *   2
     *   3
     **/

    KisForest<int> forest1;

    auto it0 = forest1.insert(childBegin(forest1), 0);

    auto it1 = forest1.insert(childEnd(it0), 1);
    auto it2 = forest1.insert(childEnd(it0), 2);
    auto it3 = forest1.insert(childEnd(it0), 3);

    QVERIFY(testForestIteration(begin(forest1),
                                end(forest1),
                                {0, 1, 2, 3}));

    KisForest<int> forest2;

    QVERIFY(testForestIteration(begin(forest2),
                                end(forest2),
                                {}));


    forest1.swap(forest2);

    QVERIFY(testForestIteration(begin(forest1),
                                end(forest1),
                                {}));

    QVERIFY(testForestIteration(begin(forest2),
                                end(forest2),
                                {0, 1, 2, 3}));

    /**
     * After swapping the containers the iterators are guaranteed to be valid still,
     * as per normal C++ convention.
     */
    QCOMPARE(childBegin(forest2), it0);
    QCOMPARE(childBegin(it0), it1);
    QCOMPARE(std::next(childBegin(it0)), it2);
    QCOMPARE(std::next(std::next(childBegin(it0))), it3);
}

void KisForestTest::testForestEmpty()
{
    KisForest<int> forest;

    /**
     * 0 1
     *   2
     *   3
     **/


    QVERIFY(forest.empty());

    auto it0 = forest.insert(childBegin(forest), 0);

    [[maybe_unused]] auto it1 = forest.insert(childEnd(it0), 1);
    [[maybe_unused]] auto it2 = forest.insert(childEnd(it0), 2);
    [[maybe_unused]] auto it3 = forest.insert(childEnd(it0), 3);

    QVERIFY(testForestIteration(begin(forest),
                                end(forest),
                                {0, 1, 2, 3}));

    QVERIFY(!forest.empty());

    forest.erase(it0);

    QVERIFY(forest.empty());
}

void KisForestTest::testSiblingsOnEndIterator()
{
    KisForest<int> forest;

    QVERIFY(childBegin(forest) == childEnd(forest));

    // toplevel **end** iterator on an empty forest
    {
        auto toplevelEndIt = childEnd(forest);

        QVERIFY(toplevelEndIt == siblingBegin(toplevelEndIt));
        QVERIFY(toplevelEndIt == siblingCurrent(toplevelEndIt));
        QVERIFY(toplevelEndIt == siblingEnd(toplevelEndIt));
    }


    auto it0 = forest.insert(childEnd(forest), 0);

    // toplevel iterators on a non-empty forest
    {
        QVERIFY(it0 == siblingBegin(it0));
        QVERIFY(it0 == siblingCurrent(it0));
        QVERIFY(childEnd(forest) == siblingEnd(it0));
    }

    // toplevel **end** iterator on a non-empty forest
    {
        QVERIFY(it0 == siblingBegin(childEnd(forest)));
        QVERIFY(childEnd(forest) == siblingCurrent(childEnd(forest)));
        QVERIFY(childEnd(forest) == siblingEnd(childEnd(forest)));
    }

    auto subordinateEnd = childEnd(it0);

    // subordinate **end** iterators on an empty subtree
    {
        QVERIFY(subordinateEnd == siblingBegin(subordinateEnd));
        QVERIFY(subordinateEnd == siblingCurrent(subordinateEnd));
        QVERIFY(subordinateEnd == siblingEnd(subordinateEnd));
    }

    auto parentEnd = forest.parentEnd();

    // iterators on the parentEnd iterator
    {
        QVERIFY(parentEnd == siblingBegin(parentEnd));
        QVERIFY(parentEnd == siblingCurrent(parentEnd));
        QVERIFY(parentEnd == siblingEnd(parentEnd));
    }
}

void KisForestTest::testParentIterator()
{
    KisForest<int> forest;

    auto nonExistentEnd = childEnd(forest);
    QVERIFY(childBegin(nonExistentEnd) == childEnd(nonExistentEnd));
    QVERIFY(forest.parentEnd() == KisForestDetail::parent(nonExistentEnd));

    // dig into hierarchy of non-existent nodes
    {
        auto childOfNonExistentEnd = childEnd(nonExistentEnd);

        QVERIFY(nonExistentEnd ==
                KisForestDetail::parent(
                    childOfNonExistentEnd));

        QVERIFY(forest.parentEnd() ==
                KisForestDetail::parent(
                    KisForestDetail::parent(
                        childOfNonExistentEnd)));

        QVERIFY(childBegin(childOfNonExistentEnd) == childEnd(childOfNonExistentEnd));

        QVERIFY(nonExistentEnd != forest.parentEnd());
        QVERIFY(nonExistentEnd != childOfNonExistentEnd);
        QVERIFY(KisForestDetail::parent(childOfNonExistentEnd) != forest.parentEnd());
    }

    auto it0 = forest.insert(childEnd(forest), 0);

    // dig into hierarchy of an existent node
    {
        QVERIFY(forest.parentEnd() == KisForestDetail::parent(it0));

        QVERIFY(childBegin(it0) == childEnd(it0));

        auto childOfExistentNode = childEnd(it0);

        QVERIFY(it0 == KisForestDetail::parent(childOfExistentNode));
        QVERIFY(forest.parentEnd() ==
                KisForestDetail::parent(
                    KisForestDetail::parent(
                        childOfExistentNode)));
    }
}

void KisForestTest::testConstChildIterators()
{
    KisForest<int> forest;


    const KisForest<int> &constForest = forest;

    auto constBeginIt = constForest.constChildBegin();
    auto constEndIt = constForest.constChildEnd();

    QVERIFY(constBeginIt == constEndIt);

    QVERIFY(constBeginIt == forest.childBegin());
    QVERIFY(constEndIt == forest.childEnd());

    forest.insert(forest.childEnd(), 10);

    static_assert(std::is_same_v<decltype(*forest.childBegin()), int&>);
    static_assert(std::is_same_v<decltype(*forest.constChildBegin()), const int&>);

    static_assert(std::is_same_v<decltype(*constForest.constChildBegin()), const int&>);
    static_assert(std::is_same_v<decltype(*constForest.childBegin()), const int&>);

    static_assert(std::is_same_v<decltype(*forest.childEnd()), int&>);
    static_assert(std::is_same_v<decltype(*forest.constChildEnd()), const int&>);

    static_assert(std::is_same_v<decltype(*constForest.constChildEnd()), const int&>);
    static_assert(std::is_same_v<decltype(*constForest.childEnd()), const int&>);

    static_assert(std::is_same_v<decltype(*forest.parentEnd()), int&>);
    static_assert(std::is_same_v<decltype(*forest.constParentEnd()), const int&>);

    static_assert(std::is_same_v<decltype(*constForest.constParentEnd()), const int&>);
    static_assert(std::is_same_v<decltype(*constForest.parentEnd()), const int&>);

}

void KisForestTest::testConstHierarchyIterators()
{
    KisForest<int> forest;

    // test on end-iterator
    {
        auto constIt = forest.constChildBegin();
        auto it = forest.childBegin();

        auto hBegin = hierarchyBegin(it);
        auto hEnd = hierarchyBegin(it);

        auto hConstBegin = hierarchyBegin(constIt);
        auto hConstEnd = hierarchyBegin(constIt);

        QVERIFY(hEnd == hConstEnd);

        static_assert(std::is_same_v<decltype(*hBegin), int&>);
        static_assert(std::is_same_v<decltype(*hEnd), int&>);

        static_assert(std::is_same_v<decltype(*hConstBegin), const int&>);
        static_assert(std::is_same_v<decltype(*hConstEnd), const int&>);
    }

    // test on a real element
    {
        forest.insert(forest.childEnd(), 10);

        QVERIFY(forest.childBegin() != forest.childEnd());

        auto hBegin = hierarchyBegin(forest.childBegin());
        auto hConstBegin = hierarchyBegin(forest.constChildBegin());

        QVERIFY(hBegin == hConstBegin);
        QVERIFY(*hBegin == *hConstBegin);
    }
}

void KisForestTest::testConstSubtreeIterators()
{
    KisForest<int> forest;

           // test on end-iterator
    {
        auto constIt = forest.constBegin();
        auto it = forest.begin();

        QVERIFY(it == constIt);

        static_assert(std::is_same_v<decltype(*it), int&>);
        static_assert(std::is_same_v<decltype(*constIt), const int&>);
    }

           // test on a real element
    {
        forest.insert(forest.childEnd(), 10);

        QVERIFY(forest.begin() != forest.end());

        QVERIFY(forest.begin() == forest.constBegin());
        QVERIFY(*forest.begin() == *forest.constBegin());
    }
}

void KisForestTest::testConstTailSubtreeIterators()
{
    KisForest<int> forest;

           // test on end-iterator
    {
        auto constIt = forest.constDepthFirstTailBegin();
        auto it = forest.depthFirstTailBegin();

        QVERIFY(it == constIt);

        static_assert(std::is_same_v<decltype(*it), int&>);
        static_assert(std::is_same_v<decltype(*constIt), const int&>);
    }

           // test on a real element
    {
        forest.insert(forest.childEnd(), 10);

        QVERIFY(forest.depthFirstTailBegin() != forest.depthFirstTailEnd());

        QVERIFY(forest.depthFirstTailBegin() == forest.constDepthFirstTailBegin());
        QVERIFY(*forest.depthFirstTailBegin() == *forest.constDepthFirstTailBegin());
    }
}


void KisForestTest::testConstTailFreeStandingForestFunctions()
{
    KisForest<int> forest;
    forest.insert(forest.childEnd(), 10);

    const KisForest<int> &constForest = forest;

    auto comparePair = [] (auto beginIt, auto endIt)
    {
        static_assert(std::is_same_v<decltype(*beginIt), int&>);
        static_assert(std::is_same_v<decltype(*endIt), int&>);

        QVERIFY(beginIt != endIt);
        QCOMPARE(*beginIt, 10);
    };

    auto compareConstPair = [] (auto beginIt, auto endIt)
    {
        static_assert(std::is_same_v<decltype(*beginIt), const int&>);
        static_assert(std::is_same_v<decltype(*endIt), const int&>);

        QVERIFY(beginIt != endIt);
        QCOMPARE(*beginIt, 10);
    };


    comparePair(childBegin(forest), childEnd(forest));
    compareConstPair(childBegin(constForest), childEnd(constForest));

    comparePair(compositionBegin(forest), compositionEnd(forest));
    compareConstPair(compositionBegin(constForest), compositionEnd(constForest));

    comparePair(tailSubtreeBegin(forest), tailSubtreeEnd(forest));
    compareConstPair(tailSubtreeBegin(constForest), tailSubtreeEnd(constForest));
}

SIMPLE_TEST_MAIN(KisForestTest)
