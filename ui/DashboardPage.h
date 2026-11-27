#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QDate>
#include <QList>
#include <QString>
#include <QWidget>

namespace Ui {
class DashboardPage;
}

/**
 * @brief Lightweight item view model used by DashboardPage.
 *
 * Designed so InventoryManager records can be mapped directly into
 * DashboardPage::setFoodItems() once the backend is connected.
 */
struct DashboardFoodItem
{
    QString name;
    double quantity = 0.0;
    QString unit;
    QDate expirationDate;
};

enum class ExpirationStatus
{
    Expired,
    Critical,
    Warning,
    Safe
};

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
     * @brief Replaces displayed food items and recalculates dashboard statistics and table.
     */
    void setFoodItems(const QList<DashboardFoodItem> &items);

    /**
     * @brief Computes expiration status from an expiration date relative to today.
     */
    static ExpirationStatus calculateStatus(const QDate &expirationDate,
                                            const QDate &today = QDate::currentDate());

signals:
    void viewInventoryRequested();

private:
    void setupCardIcons();
    void setupTable();
    void applyStyles();
    void loadSampleData();
    void refreshDashboard();
    QWidget *createStatusBadge(ExpirationStatus status) const;

    Ui::DashboardPage *ui;
    QList<DashboardFoodItem> m_items;
};

#endif // DASHBOARDPAGE_H
