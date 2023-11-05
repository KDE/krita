class AngleSelector : public QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include "AngleSelector.h"
%End

public:
    explicit AngleSelector();
protected Q_SLOTS:
    QWidget* widget();

    void setAngle(qreal newAngle);
    void reset();
    qreal angle() const;
    qreal snapAngle() const;
    qreal resetAngle() const;
    int decimals() const;
    qreal maximum() const;
    qreal minimum() const;
    QString prefix() const;
    bool wrapping() const;
    QString flipOptionsMode() const;
    int widgetsHeight() const;
    QString increasingDirection() const;
    bool isUsingFlatSpinBox() const;
    void setSnapAngle(qreal newSnapAngle);
    void setResetAngle(qreal newResetAngle);
    void setDecimals(int newNumberOfDecimals);
    void setMaximum(qreal newMaximum);
    void setMinimum(qreal newMinimum);
    void setRange(qreal newMinimum, qreal newMaximum);
    void setPrefix (const QString &newPrefix);
    void setWrapping(bool newWrapping);
    void setFlipOptionsMode(QString newMode);
    void setWidgetsHeight(int newHeight);
    void setIncreasingDirection(QString newIncreasingDirection);
    void useFlatSpinBox(bool newUseFlatSpinBox);
    static qreal closestCoterminalAngleInRange(qreal angle, qreal minimum, qreal maximum, bool *ok = nullptr);
    qreal closestCoterminalAngleInRange(qreal angle, bool *ok = nullptr) const;
    static qreal flipAngle(qreal angle, Qt::Orientations orientations);
    static qreal flipAngle(qreal angle, qreal minimum, qreal maximum, Qt::Orientations, bool *ok = nullptr);
    void flip(Qt::Orientations orientations);
Q_SIGNALS:
    void angleChanged(qreal angle);
};
