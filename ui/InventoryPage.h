
#ifndef INVENTORYPAGE_H
#define INVENTORYPAGE_H

#include <QWidget>
#include <QDate>

QT_BEGIN_NAMESPACE
namespace Ui {
class InventoryPage;
}
QT_END_NAMESPACE

class InventoryPage : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryPage(QWidget *parent = nullptr);
    ~InventoryPage();

private:
    Ui::InventoryPage *ui;

    void setupInventory();
    void addInventoryRow(const QString &name,
                         const QString &category,
                         double quantity,
                         const QString &unit,
                         const QDate &expirationDate);

    void setupInventoryRowActions(int row);
    void updateInventorySummary();
};

#endif // INVENTORYPAGE_H
