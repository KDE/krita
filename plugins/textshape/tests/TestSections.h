#ifndef TESTSECTIONS_H
#define TESTSECTIONS_H

#include <QObject>
#include <QString>
#include <qtest_kde.h>

#include "../Layout.h"
#include "../TextShape.h"

class MockTextShape;
class KoTextDocumentLayout;
class KoSectionStyle;
class KoStyleManager;
class QTextDocument;
class QTextFrame;

class TestSections : public QObject
{
    Q_OBJECT

public:
    TestSections() {}

private:
    /**
     * Initialize for a new test.
     *
     * @param sectionStyles a list of section styles to use.
     */
    void initTest(const KoSectionStyle *sectionStyles);

    /**
     * Initialize for a new test. Simplified version.
     *
     * @param sectionStyle the section style to use.
     */
    void initTestSimple(KoSectionStyle *sectionStyle);

    /// Clean up after a test.
    void cleanupTest();

private slots:
    /// Common initialization for all tests.
    void init();
    /// Test very basic layout functionality.
    void testBasicLayout();
    /// Test table padding.
    void testShrinkByMargin();
    /// Test table padding.
    void testMoveByMargin();

private:
    QTextDocument *m_doc;
    QTextTable *m_table;
    KoTextDocumentLayout *m_layout;
    KoStyleManager *m_styleManager;
    Layout *m_textLayout;
    MockTextShape *m_shape;

    // Default styles for the test.
    KoSectionStyle *m_defaultSectionStyle;
};

class MockTextShape : public TextShape
{
public:
    MockTextShape() : TextShape(0)
    {
        layout = qobject_cast<KoTextDocumentLayout*>(textShapeData()->document()->documentLayout());
    }
    void paint(QPainter &painter, const KoViewConverter &converter)
    {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
    virtual void saveOdf(KoShapeSavingContext &) const {}
    virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) {
        return true;
    }
    KoTextDocumentLayout *layout;
};

#endif // TESTSECTIONS_H
