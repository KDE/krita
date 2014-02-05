#include "KoXmlManifest.h"

//Ctor
KoXmlManifest::KoXmlManifest(QString xmlName):KoXmlGenerator(xmlName)
{
    root=xmlDocument.createElement("package");
    xmlDocument.appendChild(root);
}

//Ctor
//- file : QFile associated to a XML document
KoXmlManifest::KoXmlManifest(QFile *file):KoXmlGenerator(file,"package")
{

}

//Dtor
KoXmlManifest::~KoXmlManifest()
{

}

//Used for sorting file
KoXmlManifest::TagEnum KoXmlManifest::getTagEnumValue(QString tagName)
{
    if (tagName=="brushes") {
        return Brush;
    }
    else if (tagName=="gradients") {
        return Gradient;
    }
    else if (tagName=="paintoppresets") {
        return Paintop;
    }
    else if (tagName=="palettes") {
        return Palette;
    }
    else if (tagName=="patterns") {
        return Pattern;
    }
    else if (tagName=="templates") {
        return Template;
    }
    else if (tagName=="workspaces") {
        return Workspace;
    }
    else if (tagName=="ref") {
        return Reference;
    }
    else {
        return Other;
    }
}

//Check/sort the file to be easily comprehensible
//Any tag which doesn't exist in the enum TagList will be considered as Other
//and will be placed at the end of the file
void KoXmlManifest::checkSort()
{
    QDomNode prev;
    QDomNode current = root.firstChild();
    QDomNode next = current.nextSibling();

    TagEnum tagName;
    TagEnum lastOk=getTagEnumValue(current.toElement().tagName());

    while (!next.isNull()) {
        tagName=getTagEnumValue(next.toElement().tagName());

        if (lastOk>tagName) {
            prev=current.previousSibling();

            while (getTagEnumValue(prev.toElement().tagName())>tagName && !prev.isNull()) {
                  prev=prev.previousSibling();
            }

            if (getTagEnumValue(prev.toElement().tagName())==tagName) {
                merge(prev,next);
            }
            else if (prev.isNull()){
                root.insertBefore(next,prev);
            }
            else {
                root.insertAfter(next,prev);
            }
        }
        else if (lastOk==tagName) {
            merge(current,next);
        }
        else {
            lastOk=tagName;
            current=next;
        }

        next=current.nextSibling();
    }
}

void KoXmlManifest::merge(QDomNode dest,QDomNode src)
{
    QDomNode node=src.firstChild();
    while (!node.isNull()) {
        addTag(dest.toElement().nodeName(),node.firstChild().toText().data());
        src.removeChild(node);
        node=src.firstChild();
    }
    root.removeChild(src);
}

//Add tag with value as Text
//Replace Value if already exists
//If the file is empty (like in Ctor), bool value is set on true (default: false)
QDomElement KoXmlManifest::addTag(QString fileTypeName,QString fileName,bool emptyFile)
{
    fileTypeName=fileTypeName.toLower();

    QDomNode node;
    QDomNodeList fileTypeList=xmlDocument.elementsByTagName(fileTypeName);
    bool newNode=false;

    if (emptyFile || fileTypeList.isEmpty()) {
        node=xmlDocument.createElement(fileTypeName);
        root.appendChild(node);
        newNode=true;
    }
    else {
        node=fileTypeList.item(0);
    }

    if (emptyFile || searchValue(xmlDocument.elementsByTagName("file"),fileName).isNull()) {

        QDomElement result=node.appendChild(xmlDocument.createElement("file")).toElement();

        result.appendChild(xmlDocument.createTextNode(fileName));
        return result;
    }
    else {
        if (newNode) {
            root.removeChild(node);
        }
        return QDomElement();
    }
}

