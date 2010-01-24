#include <qtest_kde.h>
#include <kcomponentdata.h>

#include <QDebug>

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include "../dialogs/StylesModel.h"

class TestStylesModel : public QObject
{
    Q_OBJECT
public slots:
    void init();
    void cleanup();

private slots:
    void testPrecalcCache();
    void testSetManager();

private:
    void fillManager();
    KoStyleManager *manager;
};

class MockModel : public StylesModel
{
public:
    MockModel(KoStyleManager *manager, QObject *parent = 0)
            : StylesModel(manager, parent) { }

    void publicRecalculate() {
        StylesModel::recalculate();
    }
    QList<int> rootStyleIds() {
        return m_styleList;
    }
    QMultiHash<int, int> relations() {
        return m_relations;
    }
};

void TestStylesModel::init()
{
    manager = new KoStyleManager();
}

void TestStylesModel::cleanup()
{
    delete manager;
}

void TestStylesModel::testPrecalcCache()
{
    fillManager();
    MockModel model(manager);
    QCOMPARE(model.rootStyleIds().count(), 5);

    KoParagraphStyle *s = manager->paragraphStyle(model.rootStyleIds().at(0));
    QVERIFY(s);
    QCOMPARE(s->name(), QString("Default"));
    KoParagraphStyle *code = manager->paragraphStyle(model.rootStyleIds().at(2));
    QVERIFY(code);
    QCOMPARE(code->name(), QString("code"));
    KoParagraphStyle *altered = manager->paragraphStyle(model.rootStyleIds().at(1));
    QVERIFY(altered);
    QCOMPARE(altered->name(), QString("altered"));
    KoParagraphStyle *headers = manager->paragraphStyle(model.rootStyleIds().at(3));
    QVERIFY(headers);
    QCOMPARE(headers->name(), QString("headers"));

    KoCharacterStyle *red = manager->characterStyle(model.rootStyleIds().at(4));
    QVERIFY(red);
    QCOMPARE(red->name(), QString("red"));

    //only contains parent paragraph styles with links to the child.
    QVERIFY(model.relations().contains(headers->styleId()));
    QList<int> children = model.relations().values(headers->styleId());
    QCOMPARE(children.count(), 3);
    foreach(int id, children) {
        KoParagraphStyle *head = manager->paragraphStyle(id);
        QVERIFY(head);
        QVERIFY(head->name().startsWith("Head "));
    }
}

void TestStylesModel::testSetManager()
{
    MockModel model(0);
    QCOMPARE(model.rootStyleIds().count(), 0);
    fillManager();
    model.setStyleManager(manager);
    QCOMPARE(model.rootStyleIds().count(), 5);
}

void TestStylesModel::fillManager()
{
    KoParagraphStyle *ps = new KoParagraphStyle();
    ps->setName("code");
    manager->add(ps);
    ps = new KoParagraphStyle();
    ps->setName("altered");
    manager->add(ps);

    ps = new KoParagraphStyle();
    ps->setName("headers");
    KoParagraphStyle *head = new KoParagraphStyle();
    head->setParentStyle(ps);
    head->setName("Head 1");
    manager->add(head);
    head = new KoParagraphStyle();
    head->setParentStyle(ps);
    head->setName("Head 2");
    manager->add(head);
    manager->add(ps);
    head = new KoParagraphStyle();
    head->setParentStyle(ps);
    head->setName("Head 3");
    manager->add(head);

    KoCharacterStyle *style = new KoCharacterStyle();
    style->setName("red");
    manager->add(style);
}

QTEST_KDEMAIN(TestStylesModel, GUI)

#include <TestStylesModel.moc>
