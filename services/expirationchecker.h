#ifndef EXPIRATIONCHECKER_H
#define EXPIRATIONCHECKER_H

#include "models/FoodItem.h"

#include <QDate>
#include <QString>
#include <QVector>

/**
 * @brief Expiration urgency categories based on days remaining until expiration.
 */
enum class ExpirationStatus
{
    Expired,  ///< More than 0 days overdue (remainingDays < 0)
    Critical, ///< 0 to 3 days remaining
    Warning,  ///< 4 to 7 days remaining
    Safe      ///< More than 7 days remaining
};

/**
 * @brief Classified food batch summary prepared for Dashboard and Expiration Monitor UIs.
 */
struct ExpirationRecord
{
    int id = 0;
    QString name;
    QString category;
    double quantity = 0.0;
    QString unit;
    QDate expirationDate;
    int remainingDays = 0;
    ExpirationStatus status = ExpirationStatus::Safe;
};

/**
 * @brief Pre-classified food batches grouped into the four Expiration Monitor sections.
 */
struct ExpirationMonitorGroups
{
    QVector<ExpirationRecord> expired;
    QVector<ExpirationRecord> critical;
    QVector<ExpirationRecord> warning;
    QVector<ExpirationRecord> safe;
};

/**
 * @brief Centralizes all expiration date calculations, status classification, and grouping.
 */
class ExpirationChecker
{
public:
    static int getRemainingDays(const FoodItem &item);
    static int getRemainingDays(const QDate &expirationDate);

    static ExpirationStatus getStatus(const FoodItem &item);
    static ExpirationStatus getStatus(const QDate &expirationDate);

    static QString statusToString(ExpirationStatus status);

    static QVector<FoodItem> getExpiredItems(const QVector<FoodItem> &items);
    static QVector<FoodItem> getCriticalItems(const QVector<FoodItem> &items);
    static QVector<FoodItem> getWarningItems(const QVector<FoodItem> &items);
    static QVector<FoodItem> getSafeItems(const QVector<FoodItem> &items);
    static QVector<FoodItem> getExpiringSoonItems(const QVector<FoodItem> &items);

    /**
     * @brief Counts inventory records that have not expired yet.
     */
    static int getAvailableStockCount(const QVector<FoodItem> &items);

    /**
     * @brief Converts a FoodItem into a classified ExpirationRecord.
     */
    static ExpirationRecord toExpirationRecord(const FoodItem &item);

    /**
     * @brief Returns up to maxCount most urgent records sorted by expiration date (earliest first).
     */
    static QVector<ExpirationRecord> getAttentionItems(const QVector<FoodItem> &items,
                                                       int maxCount = 5);

    /**
     * @brief Classifies all records into Expired, Critical, Warning, and Safe groups,
     *        each sorted by expiration date (earliest first).
     */
    static ExpirationMonitorGroups getMonitorGroups(const QVector<FoodItem> &items);
};

#endif // EXPIRATIONCHECKER_H
