#include "TestSections.h"

#include <KoStyleManager.h>
#include <KoTextDocument.h>
#include <KoSectionStyle.h>

#include <QTextDocument>
#include <QTextFrame>
#include <QTextCursor>
#include <QPoint>
#include <QString>
#include <QPen>

void TestSections::init()
{
    m_doc = 0;
    m_table = 0;
    m_layout = 0;
    m_styleManager = 0;
    m_textLayout = 0;
    m_shape = 0;
    m_defaultSectionStyle = 0;
}

void TestSections::initTest(const KoSectionStyle *sectionStyle)
{
    // Mock shape of size 200x1000 pt.
    m_shape = new MockTextShape();
    Q_ASSERT(m_shape);
    m_shape->setSize(QSizeF(200, 1000));

    // Document layout.
    m_layout = m_shape->layout;
    Q_ASSERT(m_layout);

    // Document.
    m_doc = m_layout->document();
    Q_ASSERT(m_doc);
    m_doc->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false));

    // Layout state (layout helper).
    m_textLayout = new Layout(m_layout);
    Q_ASSERT(m_textLayout);
    m_layout->setLayout(m_textLayout);

    // Style manager.
    m_styleManager = new KoStyleManager();
    Q_ASSERT(m_styleManager);
    KoTextDocument(m_doc).setStyleManager(m_styleManager);

    // Table style.
    m_defaultSectionStyle = new KoSectionStyle();
    Q_ASSERT(m_defaultSectionStyle);
    m_defaultSectionStyle->setLeftMargin(0.0);
    m_defaultSectionStyle->setRightMargin(0.0);
    
    QString loremIpsum("Lorem ipsum dolor sit amet, XgXgectetuer adiXiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.");

    m_doc->rootFrame()->firstCursorPosition().insertText(loremIpsum);
    
    // Style.
    QTextFrameFormat sectionFormat;
    sectionStyle->applyStyle(sectionFormat);
    QTextFrame *section = m_doc->rootFrame()->firstCursorPosition().insertFrame(sectionFormat);
    
    section->firstCursorPosition().insertText(loremIpsum);
}

void TestSections::cleanupTest()
{
    delete m_styleManager;
    delete m_defaultSectionStyle;
}

void TestSections::testBasicLayout()
{
    KoSectionStyle *sectionStyle = new KoSectionStyle();
    Q_ASSERT(sectionStyle);
    sectionStyle->setLeftMargin(0.0);
    sectionStyle->setRightMargin(0.0);
    
    initTest(sectionStyle);

    m_layout->layout();

    // a block in a section with no special margins or columns should be just as wide
    // as the reference (first) block.
    QTextLayout *blockLayout = m_doc->begin().layout();
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0);

    blockLayout = m_doc->begin().next().layout();
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0);
    cleanupTest();
}

void TestSections::testShrinkByMargin()
{
    KoSectionStyle *sectionStyle = new KoSectionStyle();
    Q_ASSERT(sectionStyle);
    sectionStyle->setLeftMargin(20.0);
    sectionStyle->setRightMargin(20.0);

    initTest(sectionStyle);

    m_layout->layout();

    // a block in a section with margins (and no columns) should be exact amoung less wide
    // as the reference (first) block.
    QTextLayout *blockLayout = m_doc->begin().layout();
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0);

    blockLayout = m_doc->begin().next().layout();
    QEXPECT_FAIL("", "unimplemented", Abort);
    QCOMPARE(blockLayout->lineAt(0).width(), 160.0);
    cleanupTest();
}

void TestSections::testMoveByMargin()
{
    KoSectionStyle *sectionStyle = new KoSectionStyle();
    Q_ASSERT(sectionStyle);
    sectionStyle->setLeftMargin(20.0);
    sectionStyle->setRightMargin(-20.0);

    initTest(sectionStyle);

    m_layout->layout();

    // a block in a section with margins (and no columns) that essentially just moves the block
    // should be just as wide as the reference (first) block.
    QTextLayout *blockLayout = m_doc->begin().layout();
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0);

    blockLayout = m_doc->begin().next().layout();
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0);
    cleanupTest();
}


QTEST_KDEMAIN(TestSections, GUI)

#include <TestSections.moc>
