class Channel : QObject
{
%TypeHeaderCode
#include "Channel.h"
%End
    Channel(const Channel & __0);
public:
    virtual ~Channel();
    bool visible() const;
    void setvisible(bool value);
    QString name() const;
    int position() const;
    int channelSize() const;
    QRect bounds() const;
    QByteArray pixelData(const QRect &rect) const;
    void setPixelData(QByteArray value, const QRect &rect);
private:

};
