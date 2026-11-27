#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QButtonGroup;
class DashboardPage;
class InventoryManager;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief Main application window hosting the sidebar navigation, top header, and page stack.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    InventoryManager *inventoryManager() const;

private slots:
    void navigateToPage(int index);

private:
    enum PageIndex {
        DashboardIndex = 0,
        InventoryIndex = 1,
        ExpirationIndex = 2
    };

    void setupSidebar();
    void setupHeader();
    void setupPages();
    void applyStyles();
    void updateBreadcrumb(int index);

    Ui::MainWindow *ui;
    QButtonGroup *m_navGroup = nullptr;
    InventoryManager *m_inventoryManager = nullptr;
    DashboardPage *m_dashboardPage = nullptr;
};

#endif // MAINWINDOW_H
