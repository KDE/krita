#ifndef TESTATTRIBUTEMANAGER_H
#define TESTATTRIBUTEMANAGER_H

#include <QObject>

class AttributeManager;
class BasicElement;

class TestAttributeManager : public QObject {
Q_OBJECT
public:
    TestAttributeManager() {}

private Q_SLOTS:
    void initTestCase();

    void cleanupTestCase();

    /// Test for correct conversion of color attributes
    void testColorConversion();

private:
    AttributeManager* m_attributeManager;
    BasicElement* m_basicElement;
};

#endif
