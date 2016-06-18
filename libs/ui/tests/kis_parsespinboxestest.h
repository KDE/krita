#ifndef KISPARSESPINBOXESTEST_H
#define KISPARSESPINBOXESTEST_H

#include <QObject>

class KisParseSpinBoxesTest : public QObject
{
	Q_OBJECT

public:
	explicit KisParseSpinBoxesTest();

private Q_SLOTS:

	void testDoubleParseNormal();
	void testDoubleParseProblem();
	void testIntParseNormal();
	void testIntParseProblem();
};

#endif // KISPARSESPINBOXESTEST_H
