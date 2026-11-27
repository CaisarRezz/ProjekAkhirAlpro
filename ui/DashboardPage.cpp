#include "DashboardPage.h"
#include "ui_DashboardPage.h"
#include "inventorymanager.h"

#include <QColor>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QTableWidgetItem>

namespace {

enum class CardIconKind {
    Total,
    Available,
    ExpiringSoon,
    Expired
};

QPixmap drawCardIcon(CardIconKind kind, const QColor &color, int size = 18)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    const qreal s = size;
    switch (kind) {
    case CardIconKind::Total: {
        // Stacked layers / basket icon
        painter.drawRoundedRect(QRectF(s * 0.18, s * 0.22, s * 0.64, s * 0.56), 2.5, 2.5);
        painter.drawLine(QPointF(s * 0.18, s * 0.42), QPointF(s * 0.82, s * 0.42));
        painter.drawLine(QPointF(s * 0.36, s * 0.58), QPointF(s * 0.64, s * 0.58));
        break;
    }
    case CardIconKind::Available: {
        // Checkmark inside circle
        painter.drawEllipse(QRectF(s * 0.15, s * 0.15, s * 0.70, s * 0.70));
        painter.drawLine(QPointF(s * 0.32, s * 0.52), QPointF(s * 0.45, s * 0.65));
        painter.drawLine(QPointF(s * 0.45, s * 0.65), QPointF(s * 0.70, s * 0.37));
        break;
    }
    case CardIconKind::ExpiringSoon: {
        // Clock icon
        painter.drawEllipse(QRectF(s * 0.15, s * 0.15, s * 0.70, s * 0.70));
        painter.drawLine(QPointF(s * 0.50, s * 0.30), QPointF(s * 0.50, s * 0.52));
        painter.drawLine(QPointF(s * 0.50, s * 0.52), QPointF(s * 0.66, s * 0.60));
        break;
    }
    case CardIconKind::Expired: {
        // Triangle exclamation icon
        QPainterPath triangle;
        triangle.moveTo(s * 0.50, s * 0.16);
        triangle.lineTo(s * 0.84, s * 0.80);
        triangle.lineTo(s * 0.16, s * 0.80);
        triangle.closeSubpath();
        painter.drawPath(triangle);
        painter.drawLine(QPointF(s * 0.50, s * 0.38), QPointF(s * 0.50, s * 0.58));
        painter.drawPoint(QPointF(s * 0.50, s * 0.69));
        break;
    }
    }

    return pixmap;
}

QString formatQuantity(double quantity, const QString &unit)
{
    const QString qtyStr = QString::number(quantity, 'g', 6);
    if (unit.trimmed().isEmpty()) {
        return qtyStr;
    }
    return QStringLiteral("%1 %2").arg(qtyStr, unit.trimmed());
}

QString formatExpirationLabel(const ExpirationRecord &record)
{
    const QString dateStr = record.expirationDate.toString(QStringLiteral("MMM d, yyyy"));
    const int daysLeft = record.remainingDays;

    if (daysLeft < 0) {
        return QStringLiteral("%1 (%2d overdue)").arg(dateStr).arg(-daysLeft);
    }
    if (daysLeft == 0) {
        return QStringLiteral("%1 (Today)").arg(dateStr);
    }
    if (daysLeft == 1) {
        return QStringLiteral("%1 (Tomorrow)").arg(dateStr);
    }
    return QStringLiteral("%1 (in %2d)").arg(dateStr).arg(daysLeft);
}

} // namespace

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DashboardPage)
{
    ui->setupUi(this);

    setupCardIcons();
    setupTable();
    applyStyles();

    connect(ui->btnViewInventory, &QPushButton::clicked,
            this, &DashboardPage::viewInventoryRequested);
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setInventoryManager(InventoryManager *manager)
{
    if (m_inventoryManager == manager) {
        refreshData();
        return;
    }

    if (m_inventoryManager) {
        disconnect(m_inventoryManager, &InventoryManager::inventoryChanged,
                   this, &DashboardPage::refreshData);
    }

    m_inventoryManager = manager;

    if (m_inventoryManager) {
        connect(m_inventoryManager, &InventoryManager::inventoryChanged,
                this, &DashboardPage::refreshData);
    }

    refreshData();
}

void DashboardPage::refreshData()
{
    if (!m_inventoryManager) {
        refreshFromItems({});
        return;
    }

    refreshFromItems(m_inventoryManager->getAllItems());
}

void DashboardPage::refreshFromItems(const QVector<FoodItem> &items)
{
    const int totalItems = static_cast<int>(items.size());
    const int availableStock = ExpirationChecker::getAvailableStockCount(items);
    const int expiringSoon = static_cast<int>(ExpirationChecker::getExpiringSoonItems(items).size());
    const int expiredItems = static_cast<int>(ExpirationChecker::getExpiredItems(items).size());

    ui->lblTotalValue->setText(QString::number(totalItems));
    ui->lblAvailableValue->setText(QString::number(availableStock));
    ui->lblExpiringSoonValue->setText(QString::number(expiringSoon));
    ui->lblExpiredValue->setText(QString::number(expiredItems));

    const QVector<ExpirationRecord> attentionItems = ExpirationChecker::getAttentionItems(items, 5);
    const int rowCount = static_cast<int>(attentionItems.size());

    ui->attentionTable->clearContents();
    ui->attentionTable->setRowCount(rowCount);

    for (int row = 0; row < rowCount; ++row) {
        const ExpirationRecord &record = attentionItems.at(row);

        auto *nameItem = new QTableWidgetItem(record.name);
        QFont nameFont = nameItem->font();
        nameFont.setWeight(QFont::DemiBold);
        nameItem->setFont(nameFont);
        nameItem->setForeground(QColor("#111827"));

        auto *qtyItem = new QTableWidgetItem(formatQuantity(record.quantity, record.unit));
        qtyItem->setForeground(QColor("#4B5563"));

        auto *dateItem = new QTableWidgetItem(formatExpirationLabel(record));
        dateItem->setForeground(record.status == ExpirationStatus::Expired ? QColor("#B91C1C")
                                                                           : QColor("#4B5563"));

        ui->attentionTable->setItem(row, 0, nameItem);
        ui->attentionTable->setItem(row, 1, qtyItem);
        ui->attentionTable->setItem(row, 2, dateItem);
        ui->attentionTable->setCellWidget(row, 3, createStatusBadge(record.status));
    }
}

void DashboardPage::setupCardIcons()
{
    ui->lblTotalIcon->setPixmap(drawCardIcon(CardIconKind::Total, QColor("#374151")));
    ui->lblAvailableIcon->setPixmap(drawCardIcon(CardIconKind::Available, QColor("#2E8B57")));
    ui->lblExpiringSoonIcon->setPixmap(drawCardIcon(CardIconKind::ExpiringSoon, QColor("#D97706")));
    ui->lblExpiredIcon->setPixmap(drawCardIcon(CardIconKind::Expired, QColor("#DC2626")));
}

void DashboardPage::setupTable()
{
    ui->attentionTable->verticalHeader()->setVisible(false);
    ui->attentionTable->verticalHeader()->setDefaultSectionSize(48);
    ui->attentionTable->setFocusPolicy(Qt::NoFocus);
    ui->attentionTable->setAlternatingRowColors(false);

    QHeaderView *header = ui->attentionTable->horizontalHeader();
    header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    header->setHighlightSections(false);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::Fixed);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::Fixed);

    ui->attentionTable->setColumnWidth(1, 140);
    ui->attentionTable->setColumnWidth(3, 130);
}

QWidget *DashboardPage::createStatusBadge(ExpirationStatus status) const
{
    const QString text = ExpirationChecker::statusToString(status);
    QString textColor;
    QString bgColor;
    QString borderColor;

    switch (status) {
    case ExpirationStatus::Expired:
        textColor = QStringLiteral("#B91C1C");
        bgColor = QStringLiteral("#FEE2E2");
        borderColor = QStringLiteral("#FCA5A5");
        break;
    case ExpirationStatus::Critical:
        textColor = QStringLiteral("#C2410C");
        bgColor = QStringLiteral("#FFEDD5");
        borderColor = QStringLiteral("#FDBA74");
        break;
    case ExpirationStatus::Warning:
        textColor = QStringLiteral("#A16207");
        bgColor = QStringLiteral("#FEF9C3");
        borderColor = QStringLiteral("#FDE047");
        break;
    case ExpirationStatus::Safe:
        textColor = QStringLiteral("#15803D");
        bgColor = QStringLiteral("#DCFCE7");
        borderColor = QStringLiteral("#86EFAC");
        break;
    }

    auto *container = new QWidget();
    auto *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *badge = new QLabel(text, container);
    badge->setAlignment(Qt::AlignCenter);
    badge->setMinimumWidth(78);
    badge->setStyleSheet(QStringLiteral(
                             "QLabel {"
                             "  color: %1;"
                             "  background-color: %2;"
                             "  border: 1px solid %3;"
                             "  border-radius: 10px;"
                             "  padding: 3px 10px;"
                             "  font-size: 11px;"
                             "  font-weight: 600;"
                             "}")
                             .arg(textColor, bgColor, borderColor));

    layout->addWidget(badge);
    return container;
}

void DashboardPage::applyStyles()
{
    setStyleSheet(QStringLiteral(R"(
        QLabel#lblTitle {
            font-size: 24px;
            font-weight: 700;
            color: #111827;
        }
        QLabel#lblSubtitle {
            font-size: 13px;
            color: #6B7280;
        }
        QPushButton#btnViewInventory {
            background-color: #2E8B57;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 8px 18px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton#btnViewInventory:hover {
            background-color: #256F46;
        }
        QPushButton#btnViewInventory:pressed {
            background-color: #1F5E3B;
        }
        QFrame#cardTotal,
        QFrame#cardAvailable,
        QFrame#cardExpiringSoon,
        QFrame#cardExpired,
        QFrame#attentionCard {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 12px;
        }
        QLabel#lblTotalTitle,
        QLabel#lblAvailableTitle,
        QLabel#lblExpiringSoonTitle,
        QLabel#lblExpiredTitle {
            font-size: 13px;
            font-weight: 600;
            color: #4B5563;
        }
        QLabel#lblTotalValue,
        QLabel#lblAvailableValue,
        QLabel#lblExpiringSoonValue,
        QLabel#lblExpiredValue {
            font-size: 28px;
            font-weight: 700;
        }
        QLabel#lblTotalValue {
            color: #111827;
        }
        QLabel#lblAvailableValue {
            color: #2E8B57;
        }
        QLabel#lblExpiringSoonValue {
            color: #D97706;
        }
        QLabel#lblExpiredValue {
            color: #DC2626;
        }
        QLabel#lblTotalCaption,
        QLabel#lblAvailableCaption,
        QLabel#lblExpiringSoonCaption,
        QLabel#lblExpiredCaption {
            font-size: 12px;
            color: #6B7280;
        }
        QLabel#lblTotalIcon {
            background-color: #F3F4F6;
            border-radius: 8px;
        }
        QLabel#lblAvailableIcon {
            background-color: #E8F5EE;
            border-radius: 8px;
        }
        QLabel#lblExpiringSoonIcon {
            background-color: #FEF3C7;
            border-radius: 8px;
        }
        QLabel#lblExpiredIcon {
            background-color: #FEE2E2;
            border-radius: 8px;
        }
        QLabel#lblTableTitle {
            font-size: 16px;
            font-weight: 700;
            color: #111827;
        }
        QLabel#lblTableHint {
            font-size: 12px;
            color: #6B7280;
        }
        QTableWidget#attentionTable {
            background-color: #FFFFFF;
            border: none;
            font-size: 13px;
        }
        QTableWidget#attentionTable::item {
            padding: 6px 8px;
            border-bottom: 1px solid #F3F4F6;
        }
        QHeaderView::section {
            background-color: #F8FAFC;
            color: #6B7280;
            font-size: 12px;
            font-weight: 600;
            padding: 10px 8px;
            border: none;
            border-top: 1px solid #E5E7EB;
            border-bottom: 1px solid #E5E7EB;
        }
    )"));
}
