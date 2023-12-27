class IntParseSpinBox: public QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include "IntParseSpinBox.h"
%End

public:
    explicit IntParseSpinBox();
public Q_SLOTS:
    QSpinBox* widget();

    void stepBy(int steps);
    void setValue(int value, bool overwriteExpression = false);
    bool isLastValid() const;
    virtual QString veryCleanText() const;

Q_SIGNALS:
    void errorWhileParsing(const QString &expr);
    void noMoreParsingError() const;
};
