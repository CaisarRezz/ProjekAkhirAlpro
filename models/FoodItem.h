#ifndef FOODITEM_H
#define FOODITEM_H

#include <QDate>
#include <QString>

/**
 * @brief Represents a single food batch record in the inventory.
 *
 * Food items with different expiration dates are stored as separate FoodItem batches.
 */
struct FoodItem
{
    int id = 0;
    QString name;
    QString category;
    double quantity = 0.0;
    QString unit;
    QDate purchaseDate;
    QDate expirationDate;
};

#endif // FOODITEM_H
