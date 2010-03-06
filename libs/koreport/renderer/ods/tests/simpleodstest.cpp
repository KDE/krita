#include "KoSimpleOdsDocument.h"
#include <kdebug.h>
#include <kurl.h>
#include <QCoreApplication>
#include <QDir>

//Test ods writing

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    KoSimpleOdsDocument ods;

    KoSimpleOdsSheet *sheet = new KoSimpleOdsSheet;
    sheet->setName("Report");

    sheet->addCell(0,0,new KoSimpleOdsCell("The"));
    sheet->addCell(1,1,new KoSimpleOdsCell("Quick"));
    sheet->addCell(2,2,new KoSimpleOdsCell("Brown"));
    sheet->addCell(3,3,new KoSimpleOdsCell("Fox"));
    sheet->addCell(1,4,new KoSimpleOdsCell("1"));
    sheet->addCell(2,5,new KoSimpleOdsCell("10"));
    sheet->addCell(3,6,new KoSimpleOdsCell("100"));
    
    ods.addSheet(sheet);
    
    ods.saveDocument(QDir::homePath() + "/simpleodstestfile.ods");
    return 0;
}