class Node : QObject
{
%TypeHeaderCode
#include "Node.h"
%End
    Node(const Node & __0);
public:
    virtual ~Node();
public Q_SLOTS:
    bool alphaLocked() const;
    void setAlphaLocked(bool value);
    QString blendingMode() const;
    void setBlendingMode(QString value);
    QList<Channel *> channels() const;
    QList<Node *> childNodes() const;
    bool addChildNode(Node *child, Node *above);
    bool removeChildNode(Node *child);
    void setChildNodes(QList<Node *> nodes);
    QString colorDepth() const;
    int colorLabel() const;
    void setColorLabel(int value);
    QString colorModel() const;
    QString colorProfile() const;
    void setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile);
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
    bool visible() const;
    void setVisible(bool value);
    InfoObject * metaDataInfo() const /Factory/;
    void setMetaDataInfo(InfoObject*  value);
    Generator * generator() const /Factory/;
    void setGenerator(Generator*  value);
    Filter * filter() const /Factory/;
    void setFilter(Filter*  value);
    Transformation * transformation() const;
    void setTransformation(Transformation*  value);
    QString fileName() const;
    void setFileName(QString value);
    QByteArray pixelData(int x, int y, int w, int h) const;
    QByteArray projectionPixelData(int x, int y, int w, int h) const;
    void setPixelData(QByteArray value, int x, int y, int w, int h);
    QRect bounds() const;
    void move(int x, int y);
    void moveToParent(Node*  parent);
    bool remove();
    Node *duplicate() /Factory/;
    void save(const QString &filename, double xRes, double yRes);
    Node *mergeDown() /Factory/;
Q_SIGNALS:
private:
};
