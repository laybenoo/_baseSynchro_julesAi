#include "valuefrequencywidget.h"
#include <QRegularExpression>
#include <QMap>
#include <QVector>
#include <QPair>
#include <algorithm>

ValueFrequencyWidget::ValueFrequencyWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void ValueFrequencyWidget::setupUi()
{
    setWindowTitle(tr("Module de Comptage / Fréquences des Valeurs"));
    resize(500, 600);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Text edit for input values
    auto *lblInput = new QLabel(tr("Zone de texte des valeurs :"), this);
    mainLayout->addWidget(lblInput);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setPlaceholderText(tr("Entrez ou collez vos valeurs ici (séparées par espaces, tabulations ou retours à la ligne)..."));
    m_textEdit->setMinimumHeight(120);
    mainLayout->addWidget(m_textEdit);

    // Buttons layout
    auto *btnLayout = new QHBoxLayout();
    m_btnCalculate = new QPushButton(tr("Valider / Compter"), this);
    m_btnClear = new QPushButton(tr("Effacer"), this);

    btnLayout->addWidget(m_btnCalculate);
    btnLayout->addWidget(m_btnClear);
    mainLayout->addLayout(btnLayout);

    // Indicators layout
    auto *statsLayout = new QHBoxLayout();
    m_lblTotalValues = new QLabel(tr("Nombre total de valeurs : 0"), this);
    m_lblDistinctValues = new QLabel(tr("Nombre de valeurs différentes : 0"), this);

    statsLayout->addWidget(m_lblTotalValues);
    statsLayout->addWidget(m_lblDistinctValues);
    mainLayout->addLayout(statsLayout);

    // Frequency Table
    m_resultTable = new QTableWidget(this);
    m_resultTable->setColumnCount(2);
    QStringList headers;
    headers << tr("Valeur") << tr("Occurrences");
    m_resultTable->setHorizontalHeaderLabels(headers);
    m_resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    mainLayout->addWidget(m_resultTable);

    // Signals & Slots connections
    connect(m_btnCalculate, &QPushButton::clicked, this, &ValueFrequencyWidget::calculateFrequencies);
    connect(m_btnClear, &QPushButton::clicked, this, &ValueFrequencyWidget::clearAll);
}

void ValueFrequencyWidget::insertText(const QString &text, bool append)
{
    if (text.isEmpty()) {
        return;
    }

    if (append) {
        QString currentText = m_textEdit->toPlainText();
        if (!currentText.isEmpty() && !currentText.endsWith("\n") && !currentText.endsWith(" ")) {
            currentText += "\n";
        }
        currentText += text;
        m_textEdit->setPlainText(currentText);
    } else {
        m_textEdit->setPlainText(text);
    }

    // Move cursor to end of text
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_textEdit->setTextCursor(cursor);
}

void ValueFrequencyWidget::calculateFrequencies()
{
    QString rawText = m_textEdit->toPlainText();
    // Split by any whitespace (spaces, tabs, newlines)
    QStringList tokens = rawText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    QMap<QString, int> frequencyMap;
    for (const QString &token : tokens) {
        frequencyMap[token]++;
    }

    m_totalValues = tokens.size();
    m_distinctValues = frequencyMap.size();

    m_lblTotalValues->setText(tr("Nombre total de valeurs : %1").arg(m_totalValues));
    m_lblDistinctValues->setText(tr("Nombre de valeurs différentes : %1").arg(m_distinctValues));

    // Sort results by frequency descending, then by value
    QVector<QPair<QString, int>> sortedList;
    for (auto it = frequencyMap.constBegin(); it != frequencyMap.constEnd(); ++it) {
        sortedList.append({it.key(), it.value()});
    }

    std::sort(sortedList.begin(), sortedList.end(), [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
        if (a.second != b.second) {
            return a.second > b.second; // Descending order of count
        }
        return a.first < b.first;       // Ascending order of key
    });

    m_resultTable->setRowCount(0);
    m_resultTable->setRowCount(sortedList.size());

    for (int i = 0; i < sortedList.size(); ++i) {
        auto *valItem = new QTableWidgetItem(sortedList[i].first);
        auto *countItem = new QTableWidgetItem(QString::number(sortedList[i].second));

        valItem->setTextAlignment(Qt::AlignCenter);
        countItem->setTextAlignment(Qt::AlignCenter);

        m_resultTable->setItem(i, 0, valItem);
        m_resultTable->setItem(i, 1, countItem);
    }
}

void ValueFrequencyWidget::clearAll()
{
    m_textEdit->clear();
    m_resultTable->setRowCount(0);
    m_totalValues = 0;
    m_distinctValues = 0;
    m_lblTotalValues->setText(tr("Nombre total de valeurs : 0"));
    m_lblDistinctValues->setText(tr("Nombre de valeurs différentes : 0"));
}
