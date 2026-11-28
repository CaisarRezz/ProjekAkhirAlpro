
#ifndef ADDFOODDIALOG_H
#define ADDFOODDIALOG_H

#include <QDialog>
#include <QString>
#include <QDate>

namespace Ui {
class AddFoodDialog;
}

class AddFoodDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFoodDialog(QWidget *parent = nullptr);
    ~AddFoodDialog();

    QString getFoodName() const;
    QString getCategory() const;
    double getQuantity() const;
    QString getUnit() const;
    QDate getPurchaseDate() const;
    QDate getExpirationDate() const;

    // Mengisi dialog dengan data makanan yang sudah ada
    void setFoodData(
        const QString &name,
        const QString &category,
        double quantity,
        const QString &unit,
        const QDate &expirationDate
        );

private:
    Ui::AddFoodDialog *ui;

private slots:
    void onAddFoodClicked();
};

#endif // ADDFOODDIALOG_H
