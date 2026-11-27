#include "expirationchecker.h"

#include <algorithm>

namespace {

QVector<FoodItem> filterByStatuses(const QVector<FoodItem> &items,
                                   bool includeExpired,
                                   bool includeCritical,
                                   bool includeWarning,
                                   bool includeSafe)
{
    QVector<FoodItem> result;
    for (const FoodItem &item : items) {
        const ExpirationStatus status = ExpirationChecker::getStatus(item);
        const bool match = (status == ExpirationStatus::Expired && includeExpired)
                           || (status == ExpirationStatus::Critical && includeCritical)
                           || (status == ExpirationStatus::Warning && includeWarning)
                           || (status == ExpirationStatus::Safe && includeSafe);
        if (match) {
            result.append(item);
        }
    }
    return result;
}

void sortRecordsByExpiration(QVector<ExpirationRecord> &records)
{
    std::sort(records.begin(), records.end(),
              [](const ExpirationRecord &a, const ExpirationRecord &b) {
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

} // namespace

int ExpirationChecker::getRemainingDays(const FoodItem &item)
{
    return getRemainingDays(item.expirationDate);
}

int ExpirationChecker::getRemainingDays(const QDate &expirationDate)
{
    return static_cast<int>(QDate::currentDate().daysTo(expirationDate));
}

ExpirationStatus ExpirationChecker::getStatus(const FoodItem &item)
{
    return getStatus(item.expirationDate);
}

ExpirationStatus ExpirationChecker::getStatus(const QDate &expirationDate)
{
    const int remainingDays = getRemainingDays(expirationDate);
    if (remainingDays < 0) {
        return ExpirationStatus::Expired;
    }
    if (remainingDays <= 3) {
        return ExpirationStatus::Critical;
    }
    if (remainingDays <= 7) {
        return ExpirationStatus::Warning;
    }
    return ExpirationStatus::Safe;
}

QString ExpirationChecker::statusToString(ExpirationStatus status)
{
    switch (status) {
    case ExpirationStatus::Expired:
        return QStringLiteral("Expired");
    case ExpirationStatus::Critical:
        return QStringLiteral("Critical");
    case ExpirationStatus::Warning:
        return QStringLiteral("Warning");
    case ExpirationStatus::Safe:
        return QStringLiteral("Safe");
    }
    return QStringLiteral("Safe");
}

QVector<FoodItem> ExpirationChecker::getExpiredItems(const QVector<FoodItem> &items)
{
    return filterByStatuses(items, true, false, false, false);
}

QVector<FoodItem> ExpirationChecker::getCriticalItems(const QVector<FoodItem> &items)
{
    return filterByStatuses(items, false, true, false, false);
}

QVector<FoodItem> ExpirationChecker::getWarningItems(const QVector<FoodItem> &items)
{
    return filterByStatuses(items, false, false, true, false);
}

QVector<FoodItem> ExpirationChecker::getSafeItems(const QVector<FoodItem> &items)
{
    return filterByStatuses(items, false, false, false, true);
}

QVector<FoodItem> ExpirationChecker::getExpiringSoonItems(const QVector<FoodItem> &items)
{
    return filterByStatuses(items, false, true, true, false);
}

int ExpirationChecker::getAvailableStockCount(const QVector<FoodItem> &items)
{
    int availableCount = 0;
    for (const FoodItem &item : items) {
        if (getStatus(item) != ExpirationStatus::Expired) {
            ++availableCount;
        }
    }
    return availableCount;
}

ExpirationRecord ExpirationChecker::toExpirationRecord(const FoodItem &item)
{
    ExpirationRecord record;
    record.id = item.id;
    record.name = item.name;
    record.category = item.category;
    record.quantity = item.quantity;
    record.unit = item.unit;
    record.expirationDate = item.expirationDate;
    record.remainingDays = getRemainingDays(item.expirationDate);
    record.status = getStatus(item.expirationDate);
    return record;
}

QVector<ExpirationRecord> ExpirationChecker::getAttentionItems(const QVector<FoodItem> &items,
                                                               int maxCount)
{
    QVector<ExpirationRecord> records;
    records.reserve(items.size());

    for (const FoodItem &item : items) {
        records.append(toExpirationRecord(item));
    }

    sortRecordsByExpiration(records);

    if (maxCount >= 0 && records.size() > maxCount) {
        records.resize(maxCount);
    }
    return records;
}

ExpirationMonitorGroups ExpirationChecker::getMonitorGroups(const QVector<FoodItem> &items)
{
    ExpirationMonitorGroups groups;

    for (const FoodItem &item : items) {
        const ExpirationRecord record = toExpirationRecord(item);
        switch (record.status) {
        case ExpirationStatus::Expired:
            groups.expired.append(record);
            break;
        case ExpirationStatus::Critical:
            groups.critical.append(record);
            break;
        case ExpirationStatus::Warning:
            groups.warning.append(record);
            break;
        case ExpirationStatus::Safe:
            groups.safe.append(record);
            break;
        }
    }

    sortRecordsByExpiration(groups.expired);
    sortRecordsByExpiration(groups.critical);
    sortRecordsByExpiration(groups.warning);
    sortRecordsByExpiration(groups.safe);

    return groups;
}
