#ifndef JSONMANAGER_H
#define JSONMANAGER_H

#include "models/FoodItem.h"

#include <QString>
#include <QVector>

/**
 * @brief Handles loading and saving FoodItem records to a local inventory.json file.
 */
class JsonManager
{
public:
    /**
     * @brief Constructs a JsonManager using the default AppDataLocation/inventory.json path
     *        unless a custom file path is provided.
     */
    explicit JsonManager(const QString &filePath = QString());

    /**
     * @brief Returns the full path to the JSON storage file.
     */
    QString filePath() const;

    /**
     * @brief Loads inventory items from disk.
     *
     * Creates an empty {"items": []} file if it does not exist yet.
     * Returns false without modifying the file if JSON is invalid or corrupted.
     */
    bool loadItems(QVector<FoodItem> &outItems, QString *errorMessage = nullptr) const;

    /**
     * @brief Saves inventory items atomically using QSaveFile.
     */
    bool saveItems(const QVector<FoodItem> &items, QString *errorMessage = nullptr) const;

private:
    static QString defaultFilePath();
    bool ensureDirectoryExists(QString *errorMessage) const;

    QString m_filePath;
};

#endif // JSONMANAGER_H
