#include "tablecontextmenuhelper.h"
#include <QSet>
#include <QPair>
#include <QMap>
#include <algorithm>

TableContextMenuHelper::TableContextMenuHelper(QTableWidget *table, ValueFrequencyWidget *freqWidget)
    : QObject(table), m_table(table), m_freqWidget(freqWidget)
{
    if (m_table) {
        m_table->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_table, &QTableWidget::customContextMenuRequested,
                this, &TableContextMenuHelper::showContextMenu);
    }
}

void TableContextMenuHelper::installSelectionContextMenu(QTableWidget *table, ValueFrequencyWidget *freqWidget)
{
    if (!table) return;
    new TableContextMenuHelper(table, freqWidget);
}

QString TableContextMenuHelper::extractSelectedText(QTableWidget *table)
{
    if (!table) return QString();

    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.isEmpty()) {
        return QString();
    }

    // Deduplicate and group items by row and col
    QMap<int, QMap<int, QString>> rowColMap;
    bool hasNonEmpty = false;

    for (QTableWidgetItem *item : selected) {
        if (!item) continue;
        QString val = item->text().trimmed();
        if (!val.isEmpty()) {
            hasNonEmpty = true;
            rowColMap[item->row()][item->column()] = val;
        }
    }

    if (!hasNonEmpty || rowColMap.isEmpty()) {
        return QString();
    }

    // Build deterministic text line by line
    QStringList lines;
    for (auto rowIt = rowColMap.constBegin(); rowIt != rowColMap.constEnd(); ++rowIt) {
        QStringList rowValues;
        const auto &colMap = rowIt.value();
        for (auto colIt = colMap.constBegin(); colIt != colMap.constEnd(); ++colIt) {
            rowValues.append(colIt.value());
        }
        lines.append(rowValues.join(" "));
    }

    return lines.join("\n");
}

void TableContextMenuHelper::showContextMenu(const QPoint &pos)
{
    if (!m_table) return;

    QMenu menu(m_table);

    QAction *actSend = menu.addAction(tr("Envoyer les cellules sélectionnées au comptage"));
    QAction *actAdd = menu.addAction(tr("Ajouter au texte de comptage"));
    QAction *actReplace = menu.addAction(tr("Remplacer le texte de comptage"));

    QAction *selectedAction = menu.exec(m_table->mapToGlobal(pos));
    if (!selectedAction) return;

    if (selectedAction == actSend || selectedAction == actReplace) {
        sendToFrequencyWidget(false);
    } else if (selectedAction == actAdd) {
        sendToFrequencyWidget(true);
    }
}

void TableContextMenuHelper::sendToFrequencyWidget(bool append)
{
    if (!m_table) return;

    QList<QTableWidgetItem*> selected = m_table->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::information(m_table, tr("Information"), tr("Veuillez sélectionner au moins une cellule."));
        return;
    }

    QString text = extractSelectedText(m_table);
    if (text.isEmpty()) {
        QMessageBox::information(m_table, tr("Information"), tr("Les cellules sélectionnées sont vides."));
        return;
    }

    if (m_freqWidget) {
        m_freqWidget->insertText(text, append);
        m_freqWidget->show();
        m_freqWidget->raise();
        m_freqWidget->activateWindow();
    } else {
        QMessageBox::warning(m_table, tr("Erreur"), tr("Le module de comptage n'est pas disponible."));
    }
}
