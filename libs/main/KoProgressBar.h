#ifndef KOPROGRESSBAR_H
#define KOPROGRESSBAR_H

#include <QProgressBar>
#include <KoProgressProxy.h>
#include "komain_export.h"

/**
 * KoProgressBar is a thin wrapper around QProgressBar that also implements
 * the abstract base class KoProgressProxy. Use this class, not QProgressBar
 * to pass to KoProgressUpdater.
 */
class KOMAIN_EXPORT KoProgressBar : public QProgressBar, public KoProgressProxy
{
    Q_OBJECT
public:

    KoProgressBar(QWidget *parent = 0);

    ~KoProgressBar();

    int maximum() const;
    void setValue(int value);
    void setRange(int minimum, int maximum);
    void setFormat(const QString &format);

signals:

    void done();
};

#endif
