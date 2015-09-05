#include "TestAttributeManager.h"

#include <BasicElement.h>
#include <AttributeManager.h>
#include <KoXmlReader.h>

#include <QTest>

void TestAttributeManager::initTestCase()
{
    m_attributeManager = new AttributeManager();
    m_basicElement = new BasicElement( 0 );
}

void TestAttributeManager::cleanupTestCase()
{
    delete m_attributeManager;
    delete m_basicElement;
}

void TestAttributeManager::testColorConversion()
{
    m_basicElement->setAttribute( "color", "blue" );
    m_basicElement->setAttribute( "color1","green" );
    m_basicElement->setAttribute( "color2","#001122" );
    m_basicElement->setAttribute( "color3","transparent" );
    m_basicElement->setAttribute( "color4","#123" );
}

QTEST_MAIN(TestAttributeManager)
