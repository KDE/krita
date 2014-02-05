#ifndef KOXMLMETA_H
#define KOXMLMETA_H

#include "KoXmlGenerator.h"

class KoXmlMeta: public KoXmlGenerator
{
private:
    enum TagEnum {Name=0, Author, Created, License, Modified, Description, Tag, Other};

public:
    KoXmlMeta(QString="meta");
    KoXmlMeta(QFile*);
    KoXmlMeta(QString,QString,QString="meta");
    KoXmlMeta(QString*,QString,QString,QString="",QString="",QString="",QString="",QString="meta");
	~KoXmlMeta();

    TagEnum getTagEnumValue(QString);

    void checkSort();
    QDomElement addTag(QString,QString,bool empty=false);
};


#endif // KOXMLMETA_H

