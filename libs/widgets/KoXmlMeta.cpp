#include "KoXmlMeta.h"

//Ctor
KoXmlMeta::KoXmlMeta(QString xmlName):KoXmlGenerator(xmlName)
{
    root=xmlDocument.createElement("package");
    xmlDocument.appendChild(root);
}

//Ctor
//- file : QFile associated to a XML document
KoXmlMeta::KoXmlMeta(QFile *file):KoXmlGenerator(file)
{

}

//Ctor
KoXmlMeta::KoXmlMeta(QString name,QString license,QString xmlName):KoXmlGenerator(xmlName)
{
    root=xmlDocument.createElement("package");
    xmlDocument.appendChild(root);

    addTag("name",name,true);
    addTag("license",license,true);
}

//Ctor
//- resourceTagList : list of resource tags linked to the set of resources
KoXmlMeta::KoXmlMeta(QString* resourceTagList,QString name,QString license,QString description,
                     QString author,QString created,QString modified,QString xmlName)
                        :KoXmlGenerator(xmlName)
{
    root=xmlDocument.createElement("package");
    xmlDocument.appendChild(root);

    addTag("name",name,true);
    addTag("author",author,true);
    addTag("created",created,true);
    addTag("license",license,true);
    addTag("modified",modified,true);
    addTag("description",description,true);

    for (int i=0;i<resourceTagList->length();i++) {
        addTag("tag",resourceTagList[i],true);
    }
}

//Dtor
KoXmlMeta::~KoXmlMeta()
{

}

//Used for sorting file
KoXmlMeta::TagEnum KoXmlMeta::getTagEnumValue(QString tagName)
{
    if (tagName=="name") {
            return Name;
    }
    else if (tagName=="author") {
        return Author;
    }
    else if (tagName=="created") {
        return Created;
    }
    else if (tagName=="license") {
        return License;
    }
    else if (tagName=="last-modified") {
        return Modified;
    }
    else if (tagName=="description") {
        return Description;
    }
    else if (tagName=="tag") {
        return Tag;
    }
    else {
        return Other;
    }
}

//Check/sort the file to be easily comprehensible
//Any tag which doesn't exist in the enum TagEnum will be considered as Other
//and will be placed at the end of the file
void KoXmlMeta::checkSort()
{
    QDomNode prev;
    QDomNode current = root.firstChild();
    QDomNode next = current.nextSibling();

    TagEnum name;
    TagEnum lastOk=getTagEnumValue(current.toElement().tagName());

    while (!next.isNull()) {
        name=getTagEnumValue(next.toElement().tagName());

        if (lastOk>name) {
            prev=current.previousSibling();
            while (getTagEnumValue(prev.toElement().tagName())>name && !prev.isNull()) {
                  prev=prev.previousSibling();
            }

            if (name!=Tag && name!=Other && getTagEnumValue(prev.toElement().tagName())==name) {
                root.removeChild(next);
            }
            else if (prev.isNull()){
                root.insertBefore(next,prev);
            }
            else {
                root.insertAfter(next,prev);
            }
        }
        else if (lastOk==name && name!=Tag && name!=Other) {
            root.removeChild(next);
        }
        else {
            lastOk=name;
            current=next;
        }

        next=current.nextSibling();
    }
}

//Add tag with value as Text
//Replace Value if already exists
QDomElement KoXmlMeta::addTag(QString tagName,QString textValue,bool emptyFile)
{
    tagName=tagName.toLower();

    int tagEnumValue=getTagEnumValue(tagName);

    if (tagEnumValue!=Other && textValue=="") {
        return QDomElement();
    }
    else {
        QDomNodeList tagList=xmlDocument.elementsByTagName(tagName);
        QDomNode node=tagList.item(0);

        if (emptyFile || tagEnumValue==Other || node.isNull() || (tagEnumValue==Tag &&
             searchValue(tagList,textValue).isNull()))
              {
            QDomElement child = xmlDocument.createElement(tagName);
            root.appendChild(child);

            if (textValue!="") {
                child.appendChild(xmlDocument.createTextNode(textValue));
            }
            return child;
        }
        else if (tagEnumValue!=Tag) {
            node.firstChild().setNodeValue(textValue);
            return node.toElement();
        }
        else {
            return QDomElement();
        }
    }
}

/*
//Parcours en profondeur récursif complet
QDomNode KoXmlMeta::getFirstTag(QDomElement start,QString tagName)
{
    QDomNode n = start.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement();
        if(e.tagName()==tagName)
            return n;
        else
        {
            QDomNode res=getFirstTag(e,tagName);
            if(!res.isNull())
                return res;
        }
        n = n.nextSibling();
    }
    return n;
}*/

