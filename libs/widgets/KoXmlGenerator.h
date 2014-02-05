#ifndef KOXMLGENERATOR_H
#define KOXMLGENERATOR_H

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QtXml/QDomNodeList>

class QFile;

class KoXmlGenerator
{
protected:

    QDomDocument xmlDocument;
    QDomElement root;

public:
    KoXmlGenerator(QString);
    KoXmlGenerator(QFile*,QString="");
    virtual ~KoXmlGenerator();

    virtual void checkSort();
    virtual QDomElement addTag(QString,QString="",bool empty=false);
    bool removeFirstTag(QString,QString="");
    void removeTag(QString);
    QDomNode searchValue(QDomNodeList,QString);

    void show();
    QString toString();
    QString toFile();
};

#endif // KOXMLGENERATOR_H

