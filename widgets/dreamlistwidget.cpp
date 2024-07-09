#include "dreamlistwidget.h"
#include "QPushButton"
#include "../dream.h"
#include "../dreammanager.h"
#include "dreamlistwidgetitem.h"
#include "../mainwindow.h"

DreamListWidget* DreamListWidget::singleton = nullptr;

DreamListWidget::DreamListWidget(QWidget *parent)
    : QListWidget(parent)
{
    scroller = QScroller::scroller(viewport());

    QScrollerProperties properties = scroller->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::DragStartDistance, 0.0);
    properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QVariant::fromValue(QScrollerProperties::OvershootAlwaysOn));

    scroller->setScrollerProperties(properties);

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    scroller->grabGesture(viewport(), QScroller::TouchGesture);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
#endif

    setAttribute(Qt::WA_AcceptTouchEvents,true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setFocusPolicy(Qt::NoFocus);
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
    idToItemMap.clear();

    // reverse sort (newest created at top)
    QList<Dream> dreams = DreamManager::self()->getAllDreams();
    std::sort(dreams.rbegin(), dreams.rend());

    DreamListWidgetItem *firstItem = NULL;

    // Iterate over each dream and create a new list item widget
    foreach (auto dream, dreams) {
        if (!dream.isGenerated && !dream.displayOriginal) continue;

        // Create the custom list item widget
        DreamListWidgetItem *itemWidget = new DreamListWidgetItem(dream.id);
        idToItemMap.insert(dream.id, itemWidget);

        if (!firstItem) {
            firstItem = itemWidget;
            firstItem->setObjectName("topItem"); // for qss
        }

        connect(itemWidget, &DreamListWidgetItem::clicked, this, [=]{
            if (scroller->state() == QScroller::Inactive) {
                emit dreamClicked(dream.id);
            } else {
                itemWidget->setChecked(false);
            }
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

void DreamListWidget::uncheckItemWidgets()
{
    foreach (auto itemWidget, idToItemMap) {
        itemWidget->setChecked(false);
    }
}

DreamListWidgetItem *DreamListWidget::getDreamItem(const QString &dreamID)
{
    return idToItemMap.value(dreamID);
}

bool DreamListWidget::event(QEvent *e)
{
    return QListWidget::event(e);
}







