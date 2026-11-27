#include "inventorymanager.h"

#include <algorithm>

InventoryManager::InventoryManager(const QString &jsonFilePath, QObject *parent)
    : QObject(parent)
    , m_jsonManager(jsonFilePath)
{
}

InventoryResult InventoryManager::load()
{
    QVector<FoodItem> loadedItems;
    QString errorMessage;

    if (!m_jsonManager.loadItems(loadedItems, &errorMessage)) {
        m_loadFailed = true;
        m_lastError = errorMessage;
        return {false, errorMessage, 0};
    }

    m_items = loadedItems;
    m_loadFailed = false;
    m_lastError.clear();
    recalculateNextId();

    emit inventoryChanged();
    return {true, QStringLiteral("Inventory loaded successfully."), 0};
}

InventoryResult InventoryManager::addItem(const FoodItem &item)
{
    if (m_loadFailed) {
        return {false,
                QStringLiteral("Cannot add item while inventory storage has a load error: %1")
                    .arg(m_lastError),
                0};
    }

    QString errorMessage;
    if (!validateItem(item, &errorMessage)) {
        return {false, errorMessage, 0};
    }

    FoodItem newItem = item;
    newItem.id = m_nextId;
    newItem.name = newItem.name.trimmed();
    newItem.category = newItem.category.trimmed();
    newItem.unit = newItem.unit.trimmed();

    QVector<FoodItem> updatedItems = m_items;
    updatedItems.append(newItem);

    if (!m_jsonManager.saveItems(updatedItems, &errorMessage)) {
        m_lastError = errorMessage;
        return {false, errorMessage, 0};
    }

    m_items = updatedItems;
    ++m_nextId;
    m_lastError.clear();

    emit inventoryChanged();
    return {true, QStringLiteral("Food item added successfully."), newItem.id};
}

InventoryResult InventoryManager::updateItem(const FoodItem &item)
{
    if (m_loadFailed) {
        return {false,
                QStringLiteral("Cannot update item while inventory storage has a load error: %1")
                    .arg(m_lastError),
                item.id};
    }

    if (item.id <= 0) {
        return {false, QStringLiteral("Invalid item ID for update."), item.id};
    }

    QString errorMessage;
    if (!validateItem(item, &errorMessage)) {
        return {false, errorMessage, item.id};
    }

    int targetIndex = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == item.id) {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex < 0) {
        return {false, QStringLiteral("Food item with ID %1 was not found.").arg(item.id), item.id};
    }

    FoodItem updatedItem = item;
    updatedItem.name = updatedItem.name.trimmed();
    updatedItem.category = updatedItem.category.trimmed();
    updatedItem.unit = updatedItem.unit.trimmed();

    QVector<FoodItem> updatedItems = m_items;
    updatedItems[targetIndex] = updatedItem;

    if (!m_jsonManager.saveItems(updatedItems, &errorMessage)) {
        m_lastError = errorMessage;
        return {false, errorMessage, item.id};
    }

    m_items = updatedItems;
    m_lastError.clear();

    emit inventoryChanged();
    return {true, QStringLiteral("Food item updated successfully."), updatedItem.id};
}

InventoryResult InventoryManager::deleteItem(int id)
{
    if (m_loadFailed) {
        return {false,
                QStringLiteral("Cannot delete item while inventory storage has a load error: %1")
                    .arg(m_lastError),
                id};
    }

    int targetIndex = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex < 0) {
        return {false, QStringLiteral("Food item with ID %1 was not found.").arg(id), id};
    }

    QVector<FoodItem> updatedItems = m_items;
    updatedItems.removeAt(targetIndex);

    QString errorMessage;
    if (!m_jsonManager.saveItems(updatedItems, &errorMessage)) {
        m_lastError = errorMessage;
        return {false, errorMessage, id};
    }

    m_items = updatedItems;
    m_lastError.clear();

    emit inventoryChanged();
    return {true, QStringLiteral("Food item deleted successfully."), id};
}

QVector<FoodItem> InventoryManager::getAllItems() const
{
    return m_items;
}

QVector<FoodItem> InventoryManager::getAllItemsSortedByExpiration() const
{
    QVector<FoodItem> sorted = m_items;
    sortByExpirationDate(sorted);
    return sorted;
}

int InventoryManager::getTotalItems() const
{
    return static_cast<int>(m_items.size());
}

QVector<FoodItem> InventoryManager::searchItems(const QString &query) const
{
    const QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        return getAllItemsSortedByExpiration();
    }

    QVector<FoodItem> matches;
    for (const FoodItem &item : m_items) {
        if (item.name.contains(trimmed, Qt::CaseInsensitive)
            || item.category.contains(trimmed, Qt::CaseInsensitive)) {
            matches.append(item);
        }
    }

    sortByExpirationDate(matches);
    return matches;
}

bool InventoryManager::hasLoadError() const
{
    return m_loadFailed;
}

QString InventoryManager::lastError() const
{
    return m_lastError;
}

QString InventoryManager::storageFilePath() const
{
    return m_jsonManager.filePath();
}

bool InventoryManager::validateItem(const FoodItem &item, QString *errorMessage)
{
    if (item.name.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Food name cannot be empty.");
        }
        return false;
    }

    if (item.quantity <= 0.0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Quantity must be greater than zero.");
        }
        return false;
    }

    if (!item.purchaseDate.isValid()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Purchase date is invalid.");
        }
        return false;
    }

    if (!item.expirationDate.isValid()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Expiration date is invalid.");
        }
        return false;
    }

    if (item.expirationDate < item.purchaseDate) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Expiration date cannot be earlier than purchase date.");
        }
        return false;
    }

    return true;
}

void InventoryManager::sortByExpirationDate(QVector<FoodItem> &items)
{
    std::sort(items.begin(), items.end(),
              [](const FoodItem &a, const FoodItem &b) {
                  if (a.expirationDate != b.expirationDate) {
                      return a.expirationDate < b.expirationDate;
                  }
                  const int nameCmp = a.name.compare(b.name, Qt::CaseInsensitive);
                  if (nameCmp != 0) {
                      return nameCmp < 0;
                  }
                  return a.id < b.id;
              });
}

void InventoryManager::recalculateNextId()
{
    int maxId = 0;
    for (const FoodItem &item : m_items) {
        if (item.id > maxId) {
            maxId = item.id;
        }
    }
    m_nextId = maxId + 1;
}
