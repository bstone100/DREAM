#include "fulldreamwidget.h"
#include "../dreammanager.h"

FullDreamWidget::FullDreamWidget(QWidget *parent) : QWidget(parent) {
    backButton = new QPushButton("Back", this);
    titleLabel = new QLabel(this);
    dateTimeLabel = new QLabel(this);
    locationLabel = new QLabel(this);
    nightmareCheckBox = new QCheckBox("Nightmare", this);
    lucidCheckBox = new QCheckBox("Lucid", this);
    favoritedCheckBox = new QCheckBox("Favorited", this);
    transcriptLabel = new QLabel(this);
    transcriptLabel->setWordWrap(true);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(backButton);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(dateTimeLabel);
    mainLayout->addWidget(locationLabel);
    mainLayout->addWidget(nightmareCheckBox);
    mainLayout->addWidget(lucidCheckBox);
    mainLayout->addWidget(favoritedCheckBox);
    mainLayout->addWidget(transcriptLabel);

    setLayout(mainLayout);

    setAttribute(Qt::WA_StyledBackground);

    connect(backButton, &QPushButton::clicked, this, &FullDreamWidget::backButtonClicked);
}

void FullDreamWidget::setDream(const QString &dreamId) {
    Dream dream = DreamManager::self()->getDream(dreamId);
    if (!dream.isValid()) return;

    currentDreamID = dreamId;
    updateWidget();
}

void FullDreamWidget::updateWidget()
{
    Dream currentDream = DreamManager::self()->getDream(currentDreamID);

    titleLabel->setText(currentDream.title);
    dateTimeLabel->setText(currentDream.recordingDateTime.toString("hh:mm AP, MMM d, yyyy"));
    locationLabel->setText(currentDream.recordingLocation);
    nightmareCheckBox->setChecked(currentDream.isNightmare);
    lucidCheckBox->setChecked(currentDream.isLucid);
    favoritedCheckBox->setChecked(currentDream.isFavorited);

    if (currentDream.isGenerated) {
        transcriptLabel->setText(currentDream.revisedTranscript);
    } else {
        transcriptLabel->setText(currentDream.originalTranscript);
    }
}









