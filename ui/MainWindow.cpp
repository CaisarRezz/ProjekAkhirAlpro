#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "DashboardPage.h"
#include "inventorymanager.h"

#include <QButtonGroup>
#include <QDate>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace {

enum class NavIconType {
    Logo,
    Dashboard,
    Inventory,
    Expiration
};

QPixmap renderVectorIcon(NavIconType type, const QColor &color, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    const qreal s = size;
    switch (type) {
    case NavIconType::Logo: {
        // Simple leaf shape inside the green brand badge
        QPainterPath leaf;
        leaf.moveTo(s * 0.78, s * 0.22);
        leaf.quadTo(s * 0.25, s * 0.22, s * 0.24, s * 0.72);
        leaf.quadTo(s * 0.75, s * 0.75, s * 0.78, s * 0.22);
        painter.setBrush(QColor(255, 255, 255, 40));
        painter.drawPath(leaf);
        painter.drawLine(QPointF(s * 0.24, s * 0.76), QPointF(s * 0.62, s * 0.38));
        break;
    }
    case NavIconType::Dashboard: {
        const qreal gap = s * 0.14;
        const qreal box = s * 0.28;
        const qreal start = s * 0.15;
        painter.drawRoundedRect(QRectF(start, start, box, box), 2.5, 2.5);
        painter.drawRoundedRect(QRectF(start + box + gap, start, box, box), 2.5, 2.5);
        painter.drawRoundedRect(QRectF(start, start + box + gap, box, box), 2.5, 2.5);
        painter.drawRoundedRect(QRectF(start + box + gap, start + box + gap, box, box), 2.5, 2.5);
        break;
    }
    case NavIconType::Inventory: {
        painter.drawRoundedRect(QRectF(s * 0.18, s * 0.28, s * 0.64, s * 0.52), 2.5, 2.5);
        painter.drawLine(QPointF(s * 0.18, s * 0.44), QPointF(s * 0.82, s * 0.44));
        painter.drawLine(QPointF(s * 0.40, s * 0.56), QPointF(s * 0.60, s * 0.56));
        break;
    }
    case NavIconType::Expiration: {
        painter.drawEllipse(QRectF(s * 0.16, s * 0.16, s * 0.68, s * 0.68));
        painter.drawLine(QPointF(s * 0.50, s * 0.30), QPointF(s * 0.50, s * 0.52));
        painter.drawLine(QPointF(s * 0.50, s * 0.52), QPointF(s * 0.65, s * 0.60));
        break;
    }
    }

    return pixmap;
}

QIcon makeNavIcon(NavIconType type)
{
    QIcon icon;
    icon.addPixmap(renderVectorIcon(type, QColor("#4B5563"), 18), QIcon::Normal, QIcon::Off);
    icon.addPixmap(renderVectorIcon(type, QColor("#2E8B57"), 18), QIcon::Normal, QIcon::On);
    return icon;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(QStringLiteral("FreshGuard"));

    setupSidebar();
    setupHeader();
    setupPages();
    applyStyles();

    navigateToPage(DashboardIndex);
}

MainWindow::~MainWindow()
{
    delete ui;
}

InventoryManager *MainWindow::inventoryManager() const
{
    return m_inventoryManager;
}

void MainWindow::setupSidebar()
{
    ui->logoBadgeLabel->setPixmap(renderVectorIcon(NavIconType::Logo, Qt::white, 22));

    ui->btnDashboard->setIcon(makeNavIcon(NavIconType::Dashboard));
    ui->btnInventory->setIcon(makeNavIcon(NavIconType::Inventory));
    ui->btnExpiration->setIcon(makeNavIcon(NavIconType::Expiration));

    const QSize iconSize(18, 18);
    ui->btnDashboard->setIconSize(iconSize);
    ui->btnInventory->setIconSize(iconSize);
    ui->btnExpiration->setIconSize(iconSize);

    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);
    m_navGroup->addButton(ui->btnDashboard, DashboardIndex);
    m_navGroup->addButton(ui->btnInventory, InventoryIndex);
    m_navGroup->addButton(ui->btnExpiration, ExpirationIndex);

    connect(m_navGroup, &QButtonGroup::idClicked, this, &MainWindow::navigateToPage);
}

void MainWindow::setupHeader()
{
    const QString formattedDate = QDate::currentDate().toString(QStringLiteral("dddd, MMM d, yyyy"));
    ui->currentDateLabel->setText(formattedDate);
}

void MainWindow::setupPages()
{
    m_inventoryManager = new InventoryManager(QString(), this);
    const InventoryResult loadResult = m_inventoryManager->load();
    if (!loadResult.success) {
        QMessageBox::warning(this,
                             QStringLiteral("FreshGuard - Storage Error"),
                             loadResult.message);
    }

    m_dashboardPage = new DashboardPage(this);
    m_dashboardPage->setInventoryManager(m_inventoryManager);
    ui->dashboardContainerLayout->addWidget(m_dashboardPage);

    connect(m_dashboardPage, &DashboardPage::viewInventoryRequested, this, [this]() {
        navigateToPage(InventoryIndex);
    });
}

void MainWindow::navigateToPage(int index)
{
    if (index < 0 || index >= ui->stackedWidget->count()) {
        return;
    }

    ui->stackedWidget->setCurrentIndex(index);

    if (QAbstractButton *button = m_navGroup->button(index)) {
        button->setChecked(true);
    }

    updateBreadcrumb(index);
}

void MainWindow::updateBreadcrumb(int index)
{
    QString pageName = QStringLiteral("Dashboard");
    if (index == InventoryIndex) {
        pageName = QStringLiteral("Inventory");
    } else if (index == ExpirationIndex) {
        pageName = QStringLiteral("Expiration Monitor");
    }

    ui->breadcrumbLabel->setText(
        QStringLiteral("<span style='color:#6B7280;'>FreshGuard</span>"
                       "<span style='color:#9CA3AF;'>  /  </span>"
                       "<span style='color:#111827; font-weight:600;'>%1</span>")
            .arg(pageName));
}

void MainWindow::applyStyles()
{
    setStyleSheet(QStringLiteral(R"(
        QMainWindow {
            background-color: #F8FAFC;
        }
        QFrame#sidebarFrame {
            background-color: #FFFFFF;
            border-right: 1px solid #E5E7EB;
        }
        QLabel#logoBadgeLabel {
            background-color: #2E8B57;
            border-radius: 10px;
        }
        QLabel#brandNameLabel {
            font-size: 16px;
            font-weight: 700;
            color: #111827;
        }
        QLabel#brandSubtitleLabel {
            font-size: 11px;
            color: #6B7280;
        }
        QLabel#navSectionLabel {
            font-size: 10px;
            font-weight: 700;
            letter-spacing: 0.8px;
            color: #9CA3AF;
            padding: 8px 10px 4px 10px;
        }
        QFrame#sidebarFrame QPushButton {
            text-align: left;
            padding: 0px 14px;
            border: none;
            border-radius: 8px;
            background-color: transparent;
            color: #4B5563;
            font-size: 13px;
            font-weight: 500;
        }
        QFrame#sidebarFrame QPushButton:hover {
            background-color: #F3F4F6;
            color: #111827;
        }
        QFrame#sidebarFrame QPushButton:checked {
            background-color: #E8F5EE;
            color: #2E8B57;
            font-weight: 600;
        }
        QFrame#sidebarFooterFrame {
            background-color: #F8FAFC;
            border: 1px solid #E5E7EB;
            border-radius: 10px;
        }
        QLabel#footerQuoteLabel {
            font-size: 11px;
            color: #6B7280;
            line-height: 1.4;
        }
        QWidget#mainAreaWidget {
            background-color: #F8FAFC;
        }
        QFrame#topHeaderFrame {
            background-color: #FFFFFF;
            border-bottom: 1px solid #E5E7EB;
        }
        QLabel#breadcrumbLabel {
            font-size: 13px;
        }
        QLabel#currentDateLabel {
            background-color: #F3F4F6;
            border: 1px solid #E5E7EB;
            border-radius: 8px;
            padding: 6px 12px;
            font-size: 12px;
            font-weight: 500;
            color: #374151;
        }
        QFrame#inventoryPlaceholderCard,
        QFrame#expirationPlaceholderCard {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 12px;
        }
        QLabel#inventoryPlaceholderTitle,
        QLabel#expirationPlaceholderTitle {
            font-size: 20px;
            font-weight: 700;
            color: #111827;
        }
        QLabel#inventoryPlaceholderDesc,
        QLabel#expirationPlaceholderDesc {
            font-size: 13px;
            color: #6B7280;
        }
    )"));
}
