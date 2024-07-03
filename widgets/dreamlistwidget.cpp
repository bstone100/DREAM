#include "dreamlistwidget.h"
#include "QPushButton"
#include "../dream.h"
#include "../dreammanager.h"
#include "QScroller"
#include "dreamlistwidgetitem.h"
#include "../mainwindow.h"

DreamListWidget* DreamListWidget::singleton = nullptr;

DreamListWidget::DreamListWidget(QWidget *parent)
    : QListWidget(parent)
{
    QScroller* scroller = QScroller::scroller(this);

    QScrollerProperties properties = scroller->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::DragStartDistance, 0.0);
    properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QVariant::fromValue(QScrollerProperties::OvershootAlwaysOn));

    scroller->setScrollerProperties(properties);
    scroller->grabGesture(this, QScroller::TouchGesture);
    scroller->grabGesture(this, QScroller::MiddleMouseButtonGesture);

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
#endif
}

DreamListWidget::~DreamListWidget() {}

DreamListWidget *DreamListWidget::self()
{
    if (!singleton)
        singleton = new DreamListWidget(MainWindow::self());
    return singleton;
}

void DreamListWidget::updateDreams() {
    // Clear existing items
    this->clear();

    // Get all dreams
    QList<Dream> dreams = DreamManager::self()->getAllDreams();
    std::sort(dreams.rbegin(), dreams.rend());

    // Iterate over each dream and create a new list item widget
    foreach (auto dream, dreams) {
        if (!dream.isGenerated) continue;

        // Create the custom list item widget
        DreamListWidgetItem *itemWidget = new DreamListWidgetItem(dream.id);

        connect(itemWidget, &DreamListWidgetItem::clicked, this, [=]{
            emit dreamClicked(dream.id);
        });

        // Create a QListWidgetItem and set its size
        QListWidgetItem *listItem = new QListWidgetItem(this);
        listItem->setSizeHint(itemWidget->sizeHint()); // Ensure the custom widget fits well

        // Add the QListWidgetItem to the list
        this->addItem(listItem);

        // Set the custom widget for display
        this->setItemWidget(listItem, itemWidget);
    }
}

void DreamListWidget::updateSize()
{
    int width = MainWindow::self()->width() * .95;
    setFixedWidth(width);
}

bool DreamListWidget::event(QEvent *e)
{
    return QListWidget::event(e);
}








