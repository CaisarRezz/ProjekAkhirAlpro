
#include "addfooddialog.h"
#include "ui_addfooddialog.h"

#include <QMessageBox>
#include <QDate>

AddFoodDialog::AddFoodDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddFoodDialog)
{
    ui->setupUi(this);

    // Mengisi pilihan kategori
    ui->comboCategory->addItems({
        "Vegetables",
        "Fruits",
        "Dairy",
        "Meat & Seafood",
        "Grains",
        "Others"
    });

    // Mengisi pilihan satuan
    ui->comboUnit->addItems({
        "bag",
        "cups",
        "pieces",
        "kg",
        "grams",
        "bottles",
        "packs"
    });

    // Mengatur tanggal awal
    ui->datePurchase->setDate(QDate::currentDate());
    ui->dateExpiration->setDate(QDate::currentDate());

    // Tombol Close dan Cancel
    connect(ui->btnClose, &QPushButton::clicked,
            this, &QDialog::reject);

    connect(ui->btnCancel, &QPushButton::clicked,
            this, &QDialog::reject);

    // Tombol Add Food
    connect(ui->btnAddFood, &QPushButton::clicked,
            this, &AddFoodDialog::onAddFoodClicked);
}

AddFoodDialog::~AddFoodDialog()
{
    delete ui;
}

// Validasi ketika tombol Add Food ditekan
void AddFoodDialog::onAddFoodClicked()
{
    QString foodName = ui->inputFoodName->text().trimmed();

    if (foodName.isEmpty()) {
        QMessageBox::warning(
            this,
            "Input Required",
            "Please enter the food name."
            );

        ui->inputFoodName->setFocus();
        return;
    }

    if (ui->comboCategory->currentIndex() < 0 ||
        ui->comboUnit->currentIndex() < 0) {
        QMessageBox::warning(
            this,
            "Input Required",
            "Please select a category and unit."
            );
        return;
    }

    if (ui->dateExpiration->date() <
        ui->datePurchase->date()) {
        QMessageBox::warning(
            this,
            "Invalid Date",
            "Expiration date cannot be earlier than purchase date."
            );
        return;
    }

    // Jika semua input valid, dialog berhasil diterima
    accept();
}

// Fungsi untuk mengambil data dari form
QString AddFoodDialog::getFoodName() const
{
    return ui->inputFoodName->text().trimmed();
}

QString AddFoodDialog::getCategory() const
{
    return ui->comboCategory->currentText();
}

double AddFoodDialog::getQuantity() const
{
    return ui->spinQuantity->value();
}

QString AddFoodDialog::getUnit() const
{
    return ui->comboUnit->currentText();
}

QDate AddFoodDialog::getPurchaseDate() const
{
    return ui->datePurchase->date();
}

QDate AddFoodDialog::getExpirationDate() const
{
    return ui->dateExpiration->date();
}

void AddFoodDialog::setFoodData(
    const QString &name,
    const QString &category,
    double quantity,
    const QString &unit,
    const QDate &expirationDate
    )
{
    ui->inputFoodName->setText(name);
    ui->comboCategory->setCurrentText(category);
    ui->spinQuantity->setValue(quantity);
    ui->comboUnit->setCurrentText(unit);
    ui->dateExpiration->setDate(expirationDate);

    // Mengubah judul dan tombol untuk mode Edit
    ui->lblDialogTitle->setText("Edit Food");
    ui->lblDialogSubtitle->setText("Update your food inventory details.");
    ui->btnAddFood->setText("Save Changes");
}
