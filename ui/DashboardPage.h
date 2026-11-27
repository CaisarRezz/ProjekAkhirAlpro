#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include "expirationchecker.h"
#include "models/FoodItem.h"

#include <QVector>
#include <QWidget>

class InventoryManager;

namespace Ui {
class DashboardPage;
}

/**
 * @brief Dashboard overview page displaying summary statistics and urgent items.
 */
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;

    /**
     * @brief Connects the dashboard to an InventoryManager instance for live updates.
     */
    void setInventoryManager(InventoryManager *manager);

    /**
     * @brief Updates summary cards and attention table from the provided inventory records.
     */
    void refreshFromItems(const QVector<FoodItem> &items);

public slots:
    /**
     * @brief Reloads dashboard data from the connected InventoryManager.
     */
    void refreshData();

signals:
    void viewInventoryRequested();

private:
    void setupCardIcons();
    void setupTable();
    void applyStyles();
    QWidget *createStatusBadge(ExpirationStatus status) const;

    Ui::DashboardPage *ui;
    InventoryManager *m_inventoryManager = nullptr;
};

#endif // DASHBOARDPAGE_H
