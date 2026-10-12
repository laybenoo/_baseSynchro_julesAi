#ifndef TABLECONTEXTMENUHELPER_H
#define TABLECONTEXTMENUHELPER_H

#include <QTableWidget>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include "valuefrequencywidget.h"

class TableContextMenuHelper : public QObject
{
    Q_OBJECT

public:
    explicit TableContextMenuHelper(QTableWidget *table, ValueFrequencyWidget *freqWidget);
    ~TableContextMenuHelper() override = default;

    // Helper static function to install context menu on a table widget
    static void installSelectionContextMenu(QTableWidget *table, ValueFrequencyWidget *freqWidget);

    // Static function to extract selected items content strictly ordered by row then col
    static QString extractSelectedText(QTableWidget *table);

private slots:
    void showContextMenu(const QPoint &pos);
    void sendToFrequencyWidget(bool append);

private:
    QTableWidget *m_table{nullptr};
    ValueFrequencyWidget *m_freqWidget{nullptr};
};

#endif // TABLECONTEXTMENUHELPER_H
