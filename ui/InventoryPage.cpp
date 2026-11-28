
#include "InventoryPage.h"
#include "ui_InventoryPage.h"
#include "addfooddialog.h"

#include <QDate>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QPersistentModelIndex>
#include <QAbstractItemView>
#include <QWidget>
#include <QBrush>
#include <QColor>

// Membuat kolom tanggal bisa diurutkan berdasarkan tanggal sebenarnya.
class DateTableWidgetItem : public QTableWidgetItem
{
public:
    using QTableWidgetItem::QTableWidgetItem;

    bool operator<(const QTableWidgetItem &other) const override
    {
        QDate thisDate = QDate::fromString(text(), "MMM dd, yyyy");
        QDate otherDate = QDate::fromString(other.text(), "MMM dd, yyyy");

        if (thisDate.isValid() && otherDate.isValid())
            return thisDate < otherDate;

        return QTableWidgetItem::operator<(other);
    }
};

InventoryPage::InventoryPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InventoryPage)
{
    ui->setupUi(this);
    setupInventory();
}

InventoryPage::~InventoryPage()
{
    delete ui;
}

void InventoryPage::setupInventory()
{
    // Pengaturan tabel.
    ui->tableInventory->setColumnCount(6);
    ui->tableInventory->setHorizontalHeaderLabels({
        "Food Name",
        "Category",
        "Quantity",
        "Unit",
        "Expiration Date",
        "Actions"
    });

    ui->tableInventory->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableInventory->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableInventory->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableInventory->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tableInventory->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->tableInventory->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    ui->tableInventory->setColumnWidth(5, 150);

    ui->tableInventory->verticalHeader()->setVisible(false);
    ui->tableInventory->verticalHeader()->setDefaultSectionSize(44);
    ui->tableInventory->setAlternatingRowColors(true);
    ui->tableInventory->setShowGrid(false);
    ui->tableInventory->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableInventory->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->tableInventory->setStyleSheet(
        "QTableWidget {"
        " background: white;"
        " border: none;"
        " color: #374151;"
        "}"
        "QHeaderView::section {"
        " background-color: #f9fafb;"
        " color: #6b7280;"
        " padding: 12px 8px;"
        " border: none;"
        " border-bottom: 1px solid #e5e7eb;"
        " font-weight: 600;"
        "}"
        "QTableWidget::item {"
        " padding: 8px;"
        " border-bottom: 1px solid #f3f4f6;"
        "}"
        "QTableWidget::item:selected {"
        " background-color: #dcfce7;"
        " color: #166534;"
        "}"
        );

    // Data contoh yang sebelumnya berada di MainWindow.
    addInventoryRow("Spinach bFresh", "Vegetables", 1.00,
                    "bag", QDate(2026, 11, 23));

    addInventoryRow("Greek yogurt", "Dairy", 2.00,
                    "cups", QDate(2026, 11, 24));

    addInventoryRow("Salmon fillet", "Meat & Seafood", 2.00,
                    "pieces", QDate(2026, 11, 26));

    updateInventorySummary();

    // Tombol Add Food.
    connect(ui->btnAddFood, &QPushButton::clicked, this, [this]() {
        AddFoodDialog dialog(this);

        if (dialog.exec() == QDialog::Accepted) {
            addInventoryRow(
                dialog.getFoodName(),
                dialog.getCategory(),
                dialog.getQuantity(),
                dialog.getUnit(),
                dialog.getExpirationDate()
                );

            updateInventorySummary();
        }
    });

    // Pencarian berdasarkan nama atau kategori.
    connect(ui->txtSearchFood, &QLineEdit::textChanged,
            this, [this](const QString &searchText) {

                int visibleCount = 0;
                int totalCount = ui->tableInventory->rowCount();

                for (int row = 0; row < totalCount; ++row) {
                    QString foodName =
                        ui->tableInventory->item(row, 0)->text();

                    QString category =
                        ui->tableInventory->item(row, 1)->text();

                    bool matches =
                        foodName.contains(searchText, Qt::CaseInsensitive) ||
                        category.contains(searchText, Qt::CaseInsensitive);

                    ui->tableInventory->setRowHidden(row, !matches);

                    if (matches)
                        visibleCount++;
                }

                ui->lblShowingCount->setText(
                    QString("Showing %1 of %2 items")
                        .arg(visibleCount)
                        .arg(totalCount)
                    );
            });
}

void InventoryPage::addInventoryRow(
    const QString &name,
    const QString &category,
    double quantity,
    const QString &unit,
    const QDate &expirationDate)
{
    ui->tableInventory->setSortingEnabled(false);

    int row = ui->tableInventory->rowCount();
    ui->tableInventory->insertRow(row);

    ui->tableInventory->setItem(row, 0, new QTableWidgetItem(name));
    ui->tableInventory->setItem(row, 1, new QTableWidgetItem(category));

    QTableWidgetItem *quantityItem =
        new QTableWidgetItem(QString::number(quantity, 'f', 2));

    quantityItem->setTextAlignment(Qt::AlignCenter);
    ui->tableInventory->setItem(row, 2, quantityItem);

    ui->tableInventory->setItem(row, 3, new QTableWidgetItem(unit));

    QString formattedDate =
        expirationDate.toString("MMM dd, yyyy");

    ui->tableInventory->setItem(
        row, 4, new DateTableWidgetItem(formattedDate)
        );

    setupInventoryRowActions(row);

    ui->tableInventory->setSortingEnabled(true);
    ui->tableInventory->sortItems(4, Qt::AscendingOrder);
}

void InventoryPage::setupInventoryRowActions(int row)
{
    QWidget *actionWidget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(actionWidget);

    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(6);

    QPushButton *editButton = new QPushButton("Edit");
    QPushButton *deleteButton = new QPushButton("Delete");

    editButton->setStyleSheet(
        "QPushButton {"
        " color: #16a34a;"
        " background: #f0fdf4;"
        " border: 1px solid #bbf7d0;"
        " border-radius: 5px;"
        " padding: 5px 10px;"
        "}"
        "QPushButton:hover { background: #dcfce7; }"
        );

    deleteButton->setStyleSheet(
        "QPushButton {"
        " color: #dc2626;"
        " background: #fef2f2;"
        " border: 1px solid #fecaca;"
        " border-radius: 5px;"
        " padding: 5px 10px;"
        "}"
        "QPushButton:hover { background: #fee2e2; }"
        );

    layout->addWidget(editButton);
    layout->addWidget(deleteButton);

    ui->tableInventory->setCellWidget(row, 5, actionWidget);

    QPersistentModelIndex persistentIndex(
        ui->tableInventory->model()->index(row, 0)
        );

    // Tombol Edit.
    connect(editButton, &QPushButton::clicked, this,
            [this, persistentIndex]() {

                if (!persistentIndex.isValid())
                    return;

                int currentRow = persistentIndex.row();

                QString name =
                    ui->tableInventory->item(currentRow, 0)->text();

                QString category =
                    ui->tableInventory->item(currentRow, 1)->text();

                double quantity =
                    ui->tableInventory->item(currentRow, 2)->text().toDouble();

                QString unit =
                    ui->tableInventory->item(currentRow, 3)->text();

                QDate expirationDate =
                    QDate::fromString(
                        ui->tableInventory->item(currentRow, 4)->text(),
                        "MMM dd, yyyy"
                        );

                AddFoodDialog dialog(this);

                dialog.setFoodData(
                    name, category, quantity, unit, expirationDate
                    );

                if (dialog.exec() == QDialog::Accepted) {
                    ui->tableInventory->setSortingEnabled(false);

                    ui->tableInventory->item(currentRow, 0)
                        ->setText(dialog.getFoodName());

                    ui->tableInventory->item(currentRow, 1)
                        ->setText(dialog.getCategory());

                    ui->tableInventory->item(currentRow, 2)
                        ->setText(QString::number(dialog.getQuantity(), 'f', 2));

                    ui->tableInventory->item(currentRow, 3)
                        ->setText(dialog.getUnit());

                    ui->tableInventory->item(currentRow, 4)
                        ->setText(dialog.getExpirationDate().toString("MMM dd, yyyy"));

                    ui->tableInventory->setSortingEnabled(true);
                    ui->tableInventory->sortItems(4, Qt::AscendingOrder);

                    updateInventorySummary();
                }
            });

    // Tombol Delete.
    connect(deleteButton, &QPushButton::clicked, this,
            [this, persistentIndex]() {

                if (!persistentIndex.isValid())
                    return;

                int currentRow = persistentIndex.row();

                auto reply = QMessageBox::question(
                    this,
                    "Delete Food",
                    "Are you sure you want to delete this food item?",
                    QMessageBox::StandardButton::Yes |
                        QMessageBox::StandardButton::No
                    );

                if (reply == QMessageBox::Yes) {
                    ui->tableInventory->removeRow(currentRow);
                    updateInventorySummary();
                }
            });
}

void InventoryPage::updateInventorySummary()
{
    int totalItems = ui->tableInventory->rowCount();

    ui->lblHouseholdCount->setText(
        QString("%1 items in your household").arg(totalItems)
        );

    ui->lblShowingCount->setText(
        QString("Showing all %1 items").arg(totalItems)
        );
}
