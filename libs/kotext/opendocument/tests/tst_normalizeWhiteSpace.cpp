#include <qtest.h>

#include "../KoTextLoader.h"

#define ITERATION_COUNT 1000

class tst_normalizeWhiteSpace : public QObject
{
    Q_OBJECT
private slots:
    void normalizeWhiteSpace_data();
    void normalizeWhiteSpace();
};

void tst_normalizeWhiteSpace::normalizeWhiteSpace_data()
{
    QTest::addColumn<QString>("input");
    QTest::newRow("simple") << QString("lsdjfl sdakjf lsadj flsdj lfj   sd");
    QTest::newRow("white1") << QString("              skfj hallo\t\t\t\n\nx");
    QTest::newRow("long") << QString("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    QTest::newRow("normal") << QString("Duis autem vel eum iriure dolor in hendrerit in vulputate \
             velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero \
             eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit \
             augue duis dolore te feugait nulla facilisi. Lorem ipsum dolor sit amet, consectetuer \
             adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna \
             aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation \
             ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat");
}

void tst_normalizeWhiteSpace::normalizeWhiteSpace()
{
    QFETCH(QString, input);
    QBENCHMARK {
        for (int i = 0; i < ITERATION_COUNT; ++i) {
            QString answer = KoTextLoader::normalizeWhitespace(input, true);
            answer = KoTextLoader::normalizeWhitespace(input, false);
        }
    }
}

QTEST_MAIN(tst_normalizeWhiteSpace)

#include <tst_normalizeWhiteSpace.moc>
