class DoubleParseSpinBox: public QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include "DoubleParseSpinBox.h"
%End

public:
    explicit DoubleParseSpinBox();
public Q_SLOTS:
    QDoubleSpinBox* widget();

    void stepBy(int steps);
    void setValue(double value, bool overwriteExpression = false);
    bool isLastValid() const;
    virtual QString veryCleanText() const;

Q_SIGNALS:
    void errorWhileParsing(const QString &expr);
    void noMoreParsingError() const;
};
