#include "dreamlistwidgetitem.h"
#include <QHBoxLayout>
#include "../dreammanager.h"

DreamListWidgetItem::DreamListWidgetItem(const QString &dreamID, QWidget *parent)
    : QPushButton(parent), dreamID(dreamID)
{
    Dream dream = DreamManager::self()->getDream(dreamID);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Top layout for the title
    QHBoxLayout *topLayout = new QHBoxLayout();
    titleLabel = new QLabel(dream.title, this);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();

    // Format the date and time for display
    QString dateStr = dream.recordingDateTime.toString("h:mm A, MMM d, yyyy");

    // Format recording length from milliseconds to a more readable format
    int seconds = (dream.recordingLength / 1000) % 60;
    int minutes = (dream.recordingLength / 60000) % 60;
    QString lengthStr = QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'));

    // Bottom layout for the date and length
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    dateLabel = new QLabel(dateStr, this);
    lengthLabel = new QLabel(lengthStr, this);

    bottomLayout->addWidget(dateLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(lengthLabel);

    titleLabel->setObjectName("titleLabel");
    dateLabel->setObjectName("dateLabel");
    lengthLabel->setObjectName("lengthLabel");

    // Add layouts to the main layout
    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(bottomLayout);

    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(1,13,16,13);

    setMinimumHeight(65);


    setAttribute(Qt::WA_StyledBackground);

    setCheckable(true);
    setChecked(false);
}

QString DreamListWidgetItem::getDreamID() const
{
    return dreamID;
}

QSize DreamListWidgetItem::sizeHint() const
{
    QSize hint = QPushButton::sizeHint();
    return QSize(hint.width(), minimumHeight());
}











