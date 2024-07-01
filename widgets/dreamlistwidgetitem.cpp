#include "dreamlistwidgetitem.h"
#include <QHBoxLayout>

DreamListWidgetItem::DreamListWidgetItem(const QString &title, const QString &date, const QString &length, QWidget *parent)
    : QPushButton(parent) {
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Top layout for the title
    QHBoxLayout *topLayout = new QHBoxLayout();
    titleLabel = new QLabel(title, this);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();

    // Bottom layout for the date and length
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    dateLabel = new QLabel(date, this);
    lengthLabel = new QLabel(length, this);

    bottomLayout->addWidget(dateLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(lengthLabel);

    titleLabel->setObjectName("titleLabel");
    dateLabel->setObjectName("dateLabel");
    lengthLabel->setObjectName("lengthLabel");

    // Add layouts to the main layout
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    mainLayout->setContentsMargins(14,5,14,7);

    setAttribute(Qt::WA_StyledBackground);
}











