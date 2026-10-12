#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "valuefrequencywidget.h"
#include "tablecontextmenuhelper.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    ValueFrequencyWidget* frequencyWidget() const { return m_freqWidget; }
    QTableWidget* sampleTable() const { return m_sampleTable; }

private slots:
    void openFrequencyWidget();

private:
    void setupUi();
    void populateSampleData();

    QTableWidget *m_sampleTable{nullptr};
    QPushButton *m_btnOpenFreqWidget{nullptr};
    ValueFrequencyWidget *m_freqWidget{nullptr};
};

#endif // MAINWINDOW_H
