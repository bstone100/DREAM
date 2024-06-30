#include "dreamlistwidget.h"
#include "QPushButton"
#include "../dream.h"
#include "../dreammanager.h"

DreamListWidget* DreamListWidget::singleton = nullptr;

DreamListWidget::DreamListWidget(QWidget *parent)
    : QWidget(parent), listLayout(new QVBoxLayout), scrollArea(new QScrollArea), container(new QWidget)
{
    setStyleSheet("background-color: #142539; border-radius: 10px;");
    setAttribute(Qt::WA_StyledBackground);

    setFixedWidth(300);

    container->setLayout(listLayout);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(container);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea);
    listLayout->setAlignment(Qt::AlignTop);
}

DreamListWidget::~DreamListWidget() {}

DreamListWidget *DreamListWidget::self()
{
    if (!singleton)
        singleton = new DreamListWidget();
    return singleton;
}

void DreamListWidget::updateDreams()
{
    // Clear existing dreams
    while (QLayoutItem* item = listLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            delete widget;
        }
        delete item;
    }

    // Fetch and add new dreams from DreamManager
    auto dreams = DreamManager::self()->getAllDreams();
    std::sort(dreams.begin(), dreams.end());
    foreach (auto dream, dreams)
    {
        if (!dream.isGenerated) continue;

        QPushButton *dreamItem = new QPushButton(QString("%1 - %2 (%3)").arg(dream.recordingDateTime.toString("yyyy-MM-dd"), dream.title, dream.recordingLocation));
        dreamItem->setStyleSheet("text-align: left; padding: 10px;");
        listLayout->addWidget(dreamItem);
        connect(dreamItem, &QPushButton::clicked, this, [this, id = dream.id]() {
            qDebug() << "dreamClicked: " << id;
            emit dreamClicked(id);
        });
    }
}







