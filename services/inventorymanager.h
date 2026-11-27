#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include "JsonManager.h"
#include "models/FoodItem.h"

#include <QObject>
#include <QString>
#include <QVector>

/**
 * @brief Result returned by InventoryManager operations with status and feedback message.
 */
struct InventoryResult
{
    bool success = false;
    QString message;
    int itemId = 0;
};

/**
 * @brief Manages food batch records, validation, search, sorting, and JSON persistence.
 */
class InventoryManager : public QObject
{
    Q_OBJECT

public:
    explicit InventoryManager(const QString &jsonFilePath = QString(), QObject *parent = nullptr);

    /**
     * @brief Loads inventory records from disk via JsonManager.
     */
    InventoryResult load();

    /**
     * @brief Validates and adds a new food batch with an auto-generated unique ID.
     */
    InventoryResult addItem(const FoodItem &item);

    /**
     * @brief Validates and updates an existing food batch matched by item.id.
     */
    InventoryResult updateItem(const FoodItem &item);

    /**
     * @brief Deletes the food batch matching id and persists the change.
     */
    InventoryResult deleteItem(int id);

    QVector<FoodItem> getAllItems() const;
    QVector<FoodItem> getAllItemsSortedByExpiration() const;
    int getTotalItems() const;

    /**
     * @brief Searches food batches by name or category (case-insensitive).
     */
    QVector<FoodItem> searchItems(const QString &query) const;

    bool hasLoadError() const;
    QString lastError() const;
    QString storageFilePath() const;

signals:
    void inventoryChanged();

private:
    static bool validateItem(const FoodItem &item, QString *errorMessage);
    static void sortByExpirationDate(QVector<FoodItem> &items);
    void recalculateNextId();

    JsonManager m_jsonManager;
    QVector<FoodItem> m_items;
    int m_nextId = 1;
    bool m_loadFailed = false;
    QString m_lastError;
};

#endif // INVENTORYMANAGER_H
