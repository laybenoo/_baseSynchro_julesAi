#include <QtTest/QtTest>
#include <QApplication>
#include <QTableWidget>
#include <QTableWidgetItem>
#include "valuefrequencywidget.h"
#include "tablecontextmenuhelper.h"

class TestValueFrequency : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testValueFrequencyWidgetCalculation();
    void testInsertTextAppendAndReplace();
    void testExtractSelectedTextOrderingAndEmptyHandling();
    void testSingleCellSelection();
    void testRectangularSelection();
};

void TestValueFrequency::initTestCase()
{
}

void TestValueFrequency::testValueFrequencyWidgetCalculation()
{
    ValueFrequencyWidget widget;
    widget.insertText("74 57 63 47 74 63", false);
    widget.calculateFrequencies();

    QCOMPARE(widget.totalValuesCount(), 6);
    QCOMPARE(widget.distinctValuesCount(), 4);

    QTableWidget *resTable = widget.resultTable();
    QCOMPARE(resTable->rowCount(), 4);

    // Sorted by frequency descending: 63 (2) and 74 (2), then 47 (1) and 57 (1)
    // First row: 63 -> 2 or 74 -> 2
    QString firstVal = resTable->item(0, 0)->text();
    int firstCount = resTable->item(0, 1)->text().toInt();
    QCOMPARE(firstCount, 2);

    QString secondVal = resTable->item(1, 0)->text();
    int secondCount = resTable->item(1, 1)->text().toInt();
    QCOMPARE(secondCount, 2);

    QVERIFY((firstVal == "63" && secondVal == "74") || (firstVal == "74" && secondVal == "63"));
}

void TestValueFrequency::testInsertTextAppendAndReplace()
{
    ValueFrequencyWidget widget;
    widget.insertText("10 20 30", false);
    QCOMPARE(widget.textEdit()->toPlainText(), QString("10 20 30"));

    // Replace mode
    widget.insertText("40 50", false);
    QCOMPARE(widget.textEdit()->toPlainText(), QString("40 50"));

    // Append mode
    widget.insertText("60 70", true);
    QCOMPARE(widget.textEdit()->toPlainText(), QString("40 50\n60 70"));
}

void TestValueFrequency::testExtractSelectedTextOrderingAndEmptyHandling()
{
    QTableWidget table(3, 3);

    // Populate matrix
    // Row 0: "10", "",   "30"
    // Row 1: "",   "50", ""
    // Row 2: "70", "80", "90"
    table.setItem(0, 0, new QTableWidgetItem("10"));
    table.setItem(0, 1, new QTableWidgetItem("  ")); // empty space
    table.setItem(0, 2, new QTableWidgetItem("30"));

    table.setItem(1, 0, new QTableWidgetItem(""));
    table.setItem(1, 1, new QTableWidgetItem("50"));
    table.setItem(1, 2, new QTableWidgetItem(""));

    table.setItem(2, 0, new QTableWidgetItem("70"));
    table.setItem(2, 1, new QTableWidgetItem("80"));
    table.setItem(2, 2, new QTableWidgetItem("90"));

    // Select all items in arbitrary non-linear order to verify ordering algorithm
    table.item(2, 2)->setSelected(true);
    table.item(0, 2)->setSelected(true);
    table.item(0, 0)->setSelected(true);
    table.item(1, 1)->setSelected(true);
    table.item(2, 0)->setSelected(true);

    QString extracted = TableContextMenuHelper::extractSelectedText(&table);
    QString expected = "10 30\n50\n70 90";
    QCOMPARE(extracted, expected);
}

void TestValueFrequency::testSingleCellSelection()
{
    QTableWidget table(2, 2);
    table.setItem(0, 0, new QTableWidgetItem("74"));
    table.item(0, 0)->setSelected(true);

    QString extracted = TableContextMenuHelper::extractSelectedText(&table);
    QCOMPARE(extracted, QString("74"));
}

void TestValueFrequency::testRectangularSelection()
{
    QTableWidget table(2, 2);
    table.setItem(0, 0, new QTableWidgetItem("74"));
    table.setItem(0, 1, new QTableWidgetItem("57"));
    table.setItem(1, 0, new QTableWidgetItem("63"));
    table.setItem(1, 1, new QTableWidgetItem("47"));

    table.item(0, 0)->setSelected(true);
    table.item(0, 1)->setSelected(true);
    table.item(1, 0)->setSelected(true);
    table.item(1, 1)->setSelected(true);

    QString extracted = TableContextMenuHelper::extractSelectedText(&table);
    QString expected = "74 57\n63 47";
    QCOMPARE(extracted, expected);
}

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM") && qEnvironmentVariableIsEmpty("DISPLAY")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QApplication app(argc, argv);
    TestValueFrequency tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_valuefrequency.moc"
