#include "JsonManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>
#include <QStandardPaths>

namespace {

QJsonObject itemToJson(const FoodItem &item)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), item.id);
    obj.insert(QStringLiteral("name"), item.name);
    obj.insert(QStringLiteral("category"), item.category);
    obj.insert(QStringLiteral("quantity"), item.quantity);
    obj.insert(QStringLiteral("unit"), item.unit);
    obj.insert(QStringLiteral("purchaseDate"), item.purchaseDate.toString(Qt::ISODate));
    obj.insert(QStringLiteral("expirationDate"), item.expirationDate.toString(Qt::ISODate));
    return obj;
}

bool jsonToItem(const QJsonValue &value, FoodItem &outItem, QString *errorMessage)
{
    if (!value.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid item entry in JSON: expected an object.");
        }
        return false;
    }

    const QJsonObject obj = value.toObject();
    if (!obj.contains(QStringLiteral("id")) || !obj.value(QStringLiteral("id")).isDouble()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid item entry in JSON: missing numeric 'id'.");
        }
        return false;
    }

    FoodItem item;
    item.id = obj.value(QStringLiteral("id")).toInt();
    item.name = obj.value(QStringLiteral("name")).toString();
    item.category = obj.value(QStringLiteral("category")).toString();
    item.quantity = obj.value(QStringLiteral("quantity")).toDouble();
    item.unit = obj.value(QStringLiteral("unit")).toString();
    item.purchaseDate = QDate::fromString(obj.value(QStringLiteral("purchaseDate")).toString(), Qt::ISODate);
    item.expirationDate = QDate::fromString(obj.value(QStringLiteral("expirationDate")).toString(), Qt::ISODate);

    outItem = item;
    return true;
}

} // namespace

JsonManager::JsonManager(const QString &filePath)
    : m_filePath(filePath.trimmed().isEmpty() ? defaultFilePath() : filePath)
{
}

QString JsonManager::filePath() const
{
    return m_filePath;
}

QString JsonManager::defaultFilePath()
{
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appDataDir.isEmpty()) {
        appDataDir = QDir::currentPath();
    }
    return QDir(appDataDir).filePath(QStringLiteral("inventory.json"));
}

bool JsonManager::ensureDirectoryExists(QString *errorMessage) const
{
    const QFileInfo info(m_filePath);
    const QDir dir = info.dir();
    if (dir.exists()) {
        return true;
    }

    if (!dir.mkpath(QStringLiteral("."))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to create storage directory: %1").arg(dir.absolutePath());
        }
        return false;
    }
    return true;
}

bool JsonManager::loadItems(QVector<FoodItem> &outItems, QString *errorMessage) const
{
    if (!ensureDirectoryExists(errorMessage)) {
        return false;
    }

    QFile file(m_filePath);
    if (!file.exists()) {
        outItems.clear();
        return saveItems(outItems, errorMessage);
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Unable to open inventory file '%1': %2")
                                .arg(m_filePath, file.errorString());
        }
        return false;
    }

    const QByteArray rawData = file.readAll();
    file.close();

    if (rawData.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Inventory JSON file is empty or corrupted: %1").arg(m_filePath);
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid JSON in '%1': %2")
                                .arg(m_filePath, parseError.errorString());
        }
        return false;
    }

    if (!doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid inventory JSON format: root must be a JSON object.");
        }
        return false;
    }

    const QJsonObject root = doc.object();
    if (!root.contains(QStringLiteral("items")) || !root.value(QStringLiteral("items")).isArray()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid inventory JSON format: missing 'items' array.");
        }
        return false;
    }

    const QJsonArray itemsArray = root.value(QStringLiteral("items")).toArray();
    QVector<FoodItem> parsedItems;
    parsedItems.reserve(itemsArray.size());

    for (const QJsonValue &val : itemsArray) {
        FoodItem item;
        if (!jsonToItem(val, item, errorMessage)) {
            return false;
        }
        parsedItems.append(item);
    }

    outItems = parsedItems;
    return true;
}

bool JsonManager::saveItems(const QVector<FoodItem> &items, QString *errorMessage) const
{
    if (!ensureDirectoryExists(errorMessage)) {
        return false;
    }

    QJsonArray itemsArray;
    for (const FoodItem &item : items) {
        itemsArray.append(itemToJson(item));
    }

    QJsonObject root;
    root.insert(QStringLiteral("items"), itemsArray);

    const QJsonDocument doc(root);
    const QByteArray jsonBytes = doc.toJson(QJsonDocument::Indented);

    QSaveFile saveFile(m_filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Unable to open inventory file for writing '%1': %2")
                                .arg(m_filePath, saveFile.errorString());
        }
        return false;
    }

    if (saveFile.write(jsonBytes) != jsonBytes.size()) {
        saveFile.cancelWriting();
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to write inventory data to '%1': %2")
                                .arg(m_filePath, saveFile.errorString());
        }
        return false;
    }

    if (!saveFile.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to commit inventory file '%1': %2")
                                .arg(m_filePath, saveFile.errorString());
        }
        return false;
    }

    return true;
}
