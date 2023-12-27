class SliderSpinBox: public QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include "SliderSpinBox.h"
%End

public:
    explicit SliderSpinBox();
public Q_SLOTS:
    QWidget* widget();

    int fastSliderStep() const;
    int softMinimum() const;
    int softMaximum() const;
    bool isDragging() const;
    void setValue(int newValue);
    void setRange(int newMinimum, int newMaximum, bool computeNewFastSliderStep = true);
    void setMinimum(int newMinimum, bool computeNewFastSliderStep = true);
    void setMaximum(int newMaximum, bool computeNewFastSliderStep = true);
    void setExponentRatio(qreal newExponentRatio);
    void setBlockUpdateSignalOnDrag(bool newBlockUpdateSignalOnDrag);
    void setFastSliderStep(int newFastSliderStep);
    void setSoftRange(int newSoftMinimum, int newSoftMaximum);
    void setSoftMinimum(int newSoftMinimum);
    void setSoftMaximum(int newSoftMaximum);

Q_SIGNALS:
    void draggingFinished();
};

class DoubleSliderSpinBox: public QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include "SliderSpinBox.h"
%End

public:
    explicit DoubleSliderSpinBox();
public Q_SLOTS:
    QWidget* widget();

    qreal fastSliderStep() const;
    qreal softMinimum() const;
    qreal softMaximum() const;
    bool isDragging() const;
    void setValue(qreal newValue);
    void setRange(qreal newMinimum, qreal newMaximum, int newNumberOfDecimals = 0, bool computeNewFastSliderStep = true);
    void setMinimum(qreal newMinimum, bool computeNewFastSliderStep = true);
    void setMaximum(qreal newMaximum, bool computeNewFastSliderStep = true);
    void setExponentRatio(qreal newExponentRatio);
    void setBlockUpdateSignalOnDrag(bool newBlockUpdateSignalOnDrag);
    void setFastSliderStep(qreal newFastSliderStep);
    void setSoftRange(qreal newSoftMinimum, qreal newSoftMaximum);
    void setSoftMinimum(qreal newSoftMinimum);
    void setSoftMaximum(qreal newSoftMaximum);

Q_SIGNALS:
    void draggingFinished();
};
