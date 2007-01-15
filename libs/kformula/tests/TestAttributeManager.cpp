#include "TestAttributeManager.h"

#include <BasicElement.h>
#include <AttributeManager.h>
#include <KoXmlReader.h>
#include <QColor>

namespace FormulaShape {

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
    m_attributeManager->inheritAttributes( m_basicElement );

    QVERIFY( m_attributeManager->valueOf( "color" ) == QColor( Qt::blue ) );
    QVERIFY( m_attributeManager->valueOf( "color1" ) == QColor( Qt::green ) );
    QVERIFY( m_attributeManager->valueOf( "color2" ) == QColor( "#001122" ) );
    QVERIFY( m_attributeManager->valueOf( "color3" ) == QColor( Qt::transparent ) );
    QVERIFY( m_attributeManager->valueOf( "color4" ) == QColor( "#123" ) );

    m_attributeManager->disinheritAttributes();
}

}
