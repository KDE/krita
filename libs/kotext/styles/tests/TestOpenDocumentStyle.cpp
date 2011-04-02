/*
    Copyright (C) 2011 Pierre Ducroquet <pinaraf@pinaraf.info>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "TestOpenDocumentStyle.h"
#include <KoTableColumnStyle.h>
#include <KoTableRowStyle.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoOdfLoadingContext.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>

#include <KDebug>

#include <QBuffer>
#include <QDomDocument>
#include <QDomElement>

Attribute::Attribute(const QDomElement& element)
{
    if (element.firstChildElement() != element.lastChildElement()) {
        kFatal() << "We don't handle complex attributes so far";
    }
    QDomElement content = element.firstChildElement();
    if (content.tagName() == "ref") {
        m_referenceName = content.attribute("name");
    } else {
        m_referenceName = QString::null;
    }
    m_name = element.attribute("name");
    m_values = listValuesFromNode(element);
}

QString Attribute::name()
{
    return m_name;
}

QStringList Attribute::listValues()
{
    return m_values;
}

QStringList Attribute::listValuesFromNode(const QDomElement &m_node)
{
    if (m_referenceName.isNull()) {
        // Parse the content of the attribute
        QDomElement content = m_node.firstChildElement();
        if (content.tagName() == "choice") {
            QStringList result;
            QDomElement valueChild = content.firstChildElement();
            do {
                if (valueChild.tagName() == "value") {
                    result << valueChild.text();
                } else {
                    kFatal() << "Unrecognized choice element in " << m_name << " : " << valueChild.tagName();
                }
                valueChild = valueChild.nextSiblingElement();
            } while (!valueChild.isNull());
            return result;
        } else {
            kFatal() << "Unhandled attribute value node " << content.tagName();
        }
    } else {
        if (m_referenceName == "boolean") {
            return QStringList() << "true" << "false";
        } else if (m_referenceName == "positiveLength") {
            return QStringList() << "42px" << "42pt" << "12cm";
        } else if (m_referenceName == "relativeLength") {
            return QStringList() << "42*";
        } else {
            kFatal() << "Unhandled reference " << m_referenceName;
        }
    }
    return QStringList();
}

bool Attribute::compare(const QString& initialValue, const QString& outputValue)
{
    if (initialValue == outputValue)
        return true;
    if (m_referenceName.isNull())
        return false;
    if (m_referenceName == "positiveLength") {
        return KoUnit::parseValue(initialValue) == KoUnit::parseValue(outputValue);
    }
    return false;
}


TestOpenDocumentStyle::TestOpenDocumentStyle()
    : QObject()
{
}

void TestOpenDocumentStyle::initTestCase()
{
    // Parse the relaxng file quickly
    QString fileName(SPECS_DATA_DIR "OpenDocument-schema-v1.1.rng");
    kDebug() << fileName;
    QFile specFile(fileName);
    specFile.open(QIODevice::ReadOnly);
    QDomDocument specDocument;
    specDocument.setContent(&specFile);
    
    int count = 0;
    QDomElement mainElement = specDocument.documentElement();
    QDomNode n = mainElement.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull()) {
            count++;
            m_rngRules.insertMulti(e.attribute("name"), e);
        }
        n = n.nextSibling();
    }
}

QList<Attribute*> TestOpenDocumentStyle::listAttributesFromRNGName(const QString& name)
{
    QList<Attribute*> result;
    if (!m_rngRules.contains(name))
        return result;
    QList<QDomElement> elements = m_rngRules.values(name);
    foreach (QDomElement element, elements) {
        QDomElement child = element.firstChildElement();
        do {
            if (child.tagName() == "ref") {
                result.append(listAttributesFromRNGName(child.attribute("name")));
            } else if (child.tagName() == "optional") {
                QDomElement optionChild = child.firstChildElement();
                do {
                    if (optionChild.tagName() == "attribute") {
                        result << new Attribute(optionChild);
                    } else {
                        kFatal() << "Unrecognized optional element : " << child.tagName();
                    }
                    optionChild = optionChild.nextSiblingElement();
                } while (!optionChild.isNull());
            } else {
                kFatal() << "Unrecognized element : " << child.tagName();
            }
            child = child.nextSiblingElement();
        } while (!child.isNull());
    }
    return result;
}

QByteArray TestOpenDocumentStyle::generateStyleNodeWithAttribute(const QString& styleFamily, const QString& attributeName, const QString& attributeValue)
{
    QBuffer xmlOutputBuffer;
    KoXmlWriter *xmlWriter = new KoXmlWriter(&xmlOutputBuffer);
    xmlWriter->startDocument("style:style");
    xmlWriter->startElement("style:style");
    xmlWriter->addAttribute("xmlns:style", KoXmlNS::style);
    xmlWriter->addAttribute("xmlns:fo", KoXmlNS::fo);
    xmlWriter->addAttribute("xmlns:table", KoXmlNS::table);
    xmlWriter->addAttribute("xmlns:text", KoXmlNS::text);
    xmlWriter->addAttribute("style:name", "TestStyle");
    xmlWriter->addAttribute("style:family", styleFamily);
    xmlWriter->startElement(("style:" + styleFamily + "-properties").toLatin1());
    xmlWriter->addAttribute(attributeName.toLatin1(), attributeValue);
    xmlWriter->endElement();
    xmlWriter->endElement();
    xmlWriter->endDocument();
    delete(xmlWriter);
    
    return xmlOutputBuffer.data();
}

QByteArray TestOpenDocumentStyle::generateStyleProperties(const KoGenStyle& genStyle, const QString &styleFamily)
{
    QBuffer xmlOutputBuffer;
    KoXmlWriter *xmlWriter = new KoXmlWriter(&xmlOutputBuffer);

    xmlWriter->startDocument("style:style");
    KoGenStyles genStyles;
    genStyle.writeStyle(xmlWriter, genStyles, "style:style", "SavedTestStyle", ("style:" + styleFamily + "-properties").toLatin1());
    
    xmlWriter->endDocument();
    
    delete(xmlWriter);
    return xmlOutputBuffer.data();
}

template<class T>
bool TestOpenDocumentStyle::basicTestFunction(KoGenStyle::Type family, const QString &familyName, Attribute *attribute, const QString &value)
{
    T basicStyle;
    T genStyle;
    KoOdfStylesReader stylesReader;
    KoOdfLoadingContext loadCtxt(stylesReader, 0);
    
    QByteArray xmlOutputData = this->generateStyleNodeWithAttribute(familyName, attribute->name(), value);
    KoXmlDocument *xmlReader = new KoXmlDocument;
    xmlReader->setContent(xmlOutputData, true);
    KoXmlElement mainElement = xmlReader->documentElement();
    genStyle.loadOdf(&mainElement, loadCtxt);
    
    // THAT is often poorly implemented
    //QVERIFY(not(genStyle == basicStyle));
    
    KoGenStyle styleWriter(family, familyName.toLatin1());
    genStyle.saveOdf(styleWriter);
    
    QByteArray generatedXmlOutput = generateStyleProperties(styleWriter, familyName);
    
    KoXmlDocument *generatedXmlReader = new KoXmlDocument;
    if (!generatedXmlReader->setContent(generatedXmlOutput))
        kFatal() << "Unable to set content";
    
    KoXmlElement root = generatedXmlReader->documentElement();
    KoXmlElement properties = root.firstChild().toElement();
    QString outputPropertyValue = properties.attribute(attribute->name());
    kDebug() << "Comparing " << outputPropertyValue << value;
    return attribute->compare(outputPropertyValue, value);
}

void TestOpenDocumentStyle::testTableColumnStyle_data()
{
    QList<Attribute*> attributes = listAttributesFromRNGName("style-table-column-properties-attlist");
    QTest::addColumn<Attribute*>("attribute");
    QTest::addColumn<QString>("value");
    foreach (Attribute *attribute, attributes) {
        foreach (QString value, attribute->listValues()) {
            QTest::newRow(attribute->name().toLatin1()) << attribute << value;
        }
    }
}

void TestOpenDocumentStyle::testTableColumnStyle()
{
    QFETCH(Attribute*, attribute);
    QFETCH(QString, value);
    
    QVERIFY(basicTestFunction<KoTableColumnStyle>(KoGenStyle::TableColumnStyle, "table-column", attribute, value));
}

QTEST_MAIN(TestOpenDocumentStyle)
#include <TestOpenDocumentStyle.moc>
