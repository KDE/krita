class Node : QObject
{
%TypeHeaderCode
#include "Node.h"
%End
    Node(const Node & __0);
public:
    virtual ~Node();
    bool operator==(const Node &other) const;
    bool operator!=(const Node &other) const;
public Q_SLOTS:
    //QList<Shape *> shapes() const /Factory/;
    Node *clone() const /Factory/;
    bool alphaLocked() const;
    void setAlphaLocked(bool value);
    QString blendingMode() const;
    void setBlendingMode(QString value);
    QList<Channel *> channels() const /Factory/;
    QList<Node *> childNodes() const /Factory/;
    bool addChildNode(Node *child, Node *above);
    bool removeChildNode(Node *child);
    void setChildNodes(QList<Node *> nodes);
    QString colorDepth() const;
    bool animated() const;
    void enableAnimation() const;
    //void setPinnedToTimeline(bool pinned) const;
    //bool isPinnedToTimeline() const;
    void setCollapsed(bool collapsed);
    bool collapsed() const;
    int colorLabel() const;
    void setColorLabel(int value);
    QString colorModel() const;
    QString colorProfile() const;
    bool setColorProfile(const QString &colorProfile);
    bool setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile);
    bool inheritAlpha() const;
    void setInheritAlpha(bool value);
    bool locked() const;
    void setLocked(bool value);
    QString name() const;
    void setName(QString value);
    int opacity() const;
    void setOpacity(int value);
    Node * parentNode() const  /Factory/;
    QString type() const;
    QIcon icon() const;
    bool visible() const;
    bool hasKeyframeAtTime(int frameNumber);
    void setVisible(bool value);
    QByteArray pixelData(int x, int y, int w, int h) const;
    QByteArray pixelDataAtTime(int x, int y, int w, int h, int time) const;
    QByteArray projectionPixelData(int x, int y, int w, int h) const;
    void setPixelData(QByteArray value, int x, int y, int w, int h);
    QRect bounds() const;
    void move(int x, int y);
    QPoint position() const;
    bool remove();
    Node *duplicate() /Factory/;
    void save(const QString &filename, double xRes, double yRes, const InfoObject & exportConfiguration, const QRect &exportRect = QRect());
    Node *mergeDown() /Factory/;
    void scaleNode(QPointF origin, int width, int height, QString strategy);
    void rotateNode(double radians);
    void cropNode(int x, int y, int w, int h);
    void shearNode(double angleX, double angleY);
    QImage thumbnail(int w, int h);
    QUuid uniqueId() const;
Q_SIGNALS:
private:
};
