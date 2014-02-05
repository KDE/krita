#include "KoXmlGenerator.h"
#include <QtXml/QDomElement>
#include <QFile>
#include <QTextStream>
#include <cstdlib>
#include <iostream>
using namespace std;

/*Ctor*/
KoXmlGenerator::KoXmlGenerator(QString xmlFileName):xmlDocument(xmlFileName)
{
    root=xmlDocument.documentElement();
}

/*Ctor
 *- file : QFile associated to a XML document*/
KoXmlGenerator::KoXmlGenerator(QFile *file,QString rootTag):xmlDocument(file->fileName().section('.',0,0))
{
    if (!file->open(QIODevice::ReadOnly)) {
        exit(1);
    }

    if (!xmlDocument.setContent(file)) {
        file->close();
        exit(1);
    }
    else {
        file->close();
        if (rootTag!="") {
            QDomNodeList rootList=xmlDocument.elementsByTagName(rootTag);
            if (rootList.size()!=1) {
                exit(2);
            }
            else {
                root=rootList.item(0).toElement();
            }
        }
        else {
            root=xmlDocument.documentElement();
        }
    }
}

//Dtor
KoXmlGenerator::~KoXmlGenerator()
{

}

//Check/sort the file to be easily comprehensible.
void KoXmlGenerator::checkSort()
{

}

/*Append tagName tag to the root of the document.
 *If textValue!="", the tag will have a text child having textValue as data.
 *-> Returns the created node.*/
QDomElement KoXmlGenerator::addTag(QString tagName,QString textValue,bool emptyFile)
{
    QDomElement child = xmlDocument.createElement(tagName);
    root.appendChild(child);

    if (textValue!="") {
        child.appendChild(xmlDocument.createTextNode(textValue));
    }

    return child;
}

/*Remove the first tagName tag.
 *If textValue!="", the tag must have a text child having textValue as data.*/
bool KoXmlGenerator::removeFirstTag(QString tagName,QString textValue)
{
    QDomNodeList tagList=xmlDocument.elementsByTagName(tagName);

    if (tagList.isEmpty()) {
        return false;
    }
    else {
        if (textValue=="") {
            root.removeChild(tagList.item(0));
            return true;
        }
        else {
            QDomNode node=searchValue(tagList,textValue);
            if (node.isNull()) {
                return false;
            }
            else {
                root.removeChild(node);
                return true;
            }
        }
    }
}

//Remove all the tagName tags of the Xml File.
void KoXmlGenerator::removeTag(QString tagName)
{
    QDomNodeList tagList=xmlDocument.elementsByTagName(tagName);

    if (tagList.isEmpty()) {
        return;
    }
    else {
        for (int i=0;i<tagList.size();i++) {
            root.removeChild(tagList.item(i));
        }
    }
}

/*Looks for a value among the nodes of the lists.
 *Nodes are considered as having only one child, which is Text one.
 *-> Returns the first right node, or null node if not found.*/
QDomNode KoXmlGenerator::searchValue(QDomNodeList tagList,QString textValue)
{
    QDomNode node;

    for (int i=0;i<tagList.size();i++) {
        node=tagList.item(i);
        if (node.firstChild().toText().data()==textValue) {
            return node;
        }
    }

    return QDomNode();
}

//Show the Xml file
void KoXmlGenerator::show()
{
    cout<<qPrintable(toString());
}

//Returns QString containing Xml data
QString KoXmlGenerator::toString()
{
    return xmlDocument.toString();
}

/*Copy the QDomDocument to the linked file (file is created if not existing)
 *-> Returns the name of the file generated.*/
QString KoXmlGenerator::toFile()
{
    QString xmlName=xmlDocument.doctype().name().append(".xml");
    QFile file(xmlName);

    if (!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate)) {
        return "";
    }
    else {
        QTextStream flux(&file);

        flux.setCodec("UTF-8");
        flux<<toString();
        file.close();
        return xmlName;
    }
}

