#ifndef KOXMLMANIFEST_H
#define KOXMLMANIFEST_H

#include "KoXmlGenerator.h"

class KoXmlManifest: public KoXmlGenerator
{
private:
    enum TagEnum {Brush=0, Gradient, Paintop, Palette, Pattern,
                  Template, Workspace, Reference, Other};

public:
    KoXmlManifest(QString="manifest");
    KoXmlManifest(QFile*);
    ~KoXmlManifest();

    TagEnum getTagEnumValue(QString);

    void checkSort();
    void merge(QDomNode,QDomNode);
    QDomElement addTag(QString,QString,bool empty=false);
 };


#endif // KOXMLMANIFEST_H

