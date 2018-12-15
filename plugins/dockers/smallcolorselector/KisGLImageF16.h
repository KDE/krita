#ifndef KISGLIMAGEF16_H
#define KISGLIMAGEF16_H

#include <QSharedDataPointer>
#include <openexr/half.h>
#include <boost/operators.hpp>

class QSize;

class KisGLImageF16 : public boost::equality_comparable<KisGLImageF16>
{
public:
    KisGLImageF16();
    KisGLImageF16(const QSize &size, bool clearPixels = false);
    KisGLImageF16(int width, int height, bool clearPixels = false);
    KisGLImageF16(const KisGLImageF16 &rhs);
    KisGLImageF16& operator=(const KisGLImageF16 &rhs);

    friend bool operator==(const KisGLImageF16 &lhs, const KisGLImageF16 &rhs);

    ~KisGLImageF16();

    void clearPixels();
    void resize(const QSize &size, bool clearPixels = false);

    const half* constData() const;
    half* data();

    QSize size() const;
    int width() const;
    int height() const;

    bool isNull() const;

private:
    struct Private;
    QSharedDataPointer<Private> m_d;
};

#endif // KISGLIMAGEF16_H
