#ifndef VALUEFREQUENCYWIDGET_H
#define VALUEFREQUENCYWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

class ValueFrequencyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ValueFrequencyWidget(QWidget *parent = nullptr);
    ~ValueFrequencyWidget() override = default;

    // Public method to insert or append text into the input zone
    void insertText(const QString &text, bool append = false);

    // Getters for UI elements / testing
    QTextEdit* textEdit() const { return m_textEdit; }
    QTableWidget* resultTable() const { return m_resultTable; }
    int totalValuesCount() const { return m_totalValues; }
    int distinctValuesCount() const { return m_distinctValues; }

public slots:
    void calculateFrequencies();
    void clearAll();

private:
    void setupUi();

    QTextEdit *m_textEdit{nullptr};
    QPushButton *m_btnCalculate{nullptr};
    QPushButton *m_btnClear{nullptr};
    QTableWidget *m_resultTable{nullptr};
    QLabel *m_lblTotalValues{nullptr};
    QLabel *m_lblDistinctValues{nullptr};

    int m_totalValues{0};
    int m_distinctValues{0};
};

#endif // VALUEFREQUENCYWIDGET_H
