#include "mainwindow.h"
#include <QHeaderView>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_freqWidget = new ValueFrequencyWidget();
    setupUi();
    populateSampleData();
    TableContextMenuHelper::installSelectionContextMenu(m_sampleTable, m_freqWidget);
}

void MainWindow::setupUi()
{
    setWindowTitle(tr("iPanda - Analyse & Comptage de Fréquences"));
    resize(800, 500);

    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);

    auto *topLayout = new QHBoxLayout();
    auto *lblTitle = new QLabel(tr("Tableau de démonstration iPanda (Faites un clic droit sur les cellules sélectionnées) :"), this);
    m_btnOpenFreqWidget = new QPushButton(tr("Ouvrir le module de comptage"), this);

    topLayout->addWidget(lblTitle);
    topLayout->addStretch();
    topLayout->addWidget(m_btnOpenFreqWidget);
    mainLayout->addLayout(topLayout);

    m_sampleTable = new QTableWidget(4, 4, this);
    m_sampleTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_sampleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mainLayout->addWidget(m_sampleTable);

    connect(m_btnOpenFreqWidget, &QPushButton::clicked, this, &MainWindow::openFrequencyWidget);
}

void MainWindow::populateSampleData()
{
    // Populate with example numbers from specification
    // Row 0: 74, 57, 63, 47
    // Row 1: 74, 63, 12, 10
    // Row 2: 57, 47, 74, 63
    // Row 3: 88, 99, 100, 74
    int data[4][4] = {
        {74, 57, 63, 47},
        {74, 63, 12, 10},
        {57, 47, 74, 63},
        {88, 99, 100, 74}
    };

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            auto *item = new QTableWidgetItem(QString::number(data[r][c]));
            item->setTextAlignment(Qt::AlignCenter);
            m_sampleTable->setItem(r, c, item);
        }
    }
}

void MainWindow::openFrequencyWidget()
{
    if (m_freqWidget) {
        m_freqWidget->show();
        m_freqWidget->raise();
        m_freqWidget->activateWindow();
    }
}
