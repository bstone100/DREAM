#include "fulldreamwidget.h"
#include "../dreammanager.h"
#include "svgbutton.h"
#include "../mainwindow.h"
#include "../locationmanager.h"
#include "QMessageBox"

FullDreamWidget::FullDreamWidget(QWidget *parent) : QWidget(parent)
{
    backButton = new SvgButton(this);
    backButton->setSvgPath(":/images/leftArrow.svg");
    backButton->setIconSize(QSize(30,30));
    backButton->setFixedSize(90, 50);
    backButton->setUsingAppColors(true);

    trashButton = new SvgButton(this);
    trashButton->setSvgPath(":/images/trash.svg");
    trashButton->setIconSize(QSize(30,30));
    trashButton->setFixedSize(50, 50);
    trashButton->setUsingAppColors(true);

    heartButton = new SvgButton(this);
    heartButton->setSvgPath(":/images/heart.svg");
    heartButton->setIconSize(QSize(30,30));
    heartButton->setFixedSize(50, 50);
    heartButton->setUsingAppColors(true);
    heartButton->setCheckable(true);
    heartButton->setChecked(false);

    connect(trashButton, &QPushButton::clicked, this, &FullDreamWidget::handleTrashClicked);

    titleLabel = new QLabel(this);
    dateTimeLocationLabel = new QLabel(this);

    nightmareCheckBox = new QCheckBox("Nightmare", this);
    lucidCheckBox = new QCheckBox("Lucid", this);

    transcriptLabel = new QLabel(this);

    titleLabel->setStyleSheet("font-size: 25px;");
    dateTimeLocationLabel->setStyleSheet("font-size: 13px; color: #DBD0B3;");

//    dateTimeLocationLabel->setWordWrap(true);
    transcriptLabel->setWordWrap(true);

    connect(lucidCheckBox, &QCheckBox::clicked, this, &FullDreamWidget::handleWidgetInteraction);
    connect(nightmareCheckBox, &QCheckBox::clicked, this, &FullDreamWidget::handleWidgetInteraction);
    connect(heartButton, &SvgButton::clicked, this, &FullDreamWidget::handleWidgetInteraction);

    auto topH = new QHBoxLayout;
    topH->addWidget(backButton);
    topH->addStretch();
    topH->addWidget(trashButton);
    topH->addWidget(heartButton);

    auto hBox = new QHBoxLayout;
    hBox->addWidget(nightmareCheckBox);
    hBox->addWidget(lucidCheckBox);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topH);
    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(dateTimeLocationLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(hBox);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(transcriptLabel);
    mainLayout->addStretch();

    mainLayout->setAlignment(hBox, Qt::AlignCenter);

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

    if (!currentDream.isValid()) return;

    titleLabel->setText(currentDream.title);

    QString date = currentDream.recordingDateTime.toString("MMM d, yyyy");
    QString time = currentDream.recordingDateTime.toString("h:mm A");
    QString location = LocationManager::formattedAddress(currentDream.recordingLocation);

    QString dateTimeLocation = date + " at " + time;
    if (!location.isEmpty()) {
        dateTimeLocation += " in " + location;
    }
    dateTimeLocationLabel->setText(dateTimeLocation);


    nightmareCheckBox->setChecked(currentDream.isNightmare);
    lucidCheckBox->setChecked(currentDream.isLucid);
    heartButton->setChecked(currentDream.isFavorited);
    updateHeart();

    if (currentDream.isGenerated) {
        transcriptLabel->setText(currentDream.revisedTranscript);
    } else {
        transcriptLabel->setText(currentDream.originalTranscript);
    }
}

void FullDreamWidget::handleWidgetInteraction()
{
    Dream currentDream = DreamManager::self()->getDream(currentDreamID);

    currentDream.isLucid = lucidCheckBox->isChecked();
    currentDream.isNightmare = nightmareCheckBox->isChecked();
    currentDream.isFavorited = heartButton->isChecked();
    updateHeart();

    // maybe allow other things to be edited by user

    DreamManager::self()->insertDream(currentDream);
    MainWindow::self()->updateWidgets();
}

QString FullDreamWidget::getCurrentDreamID() const
{
    return currentDreamID;
}

void FullDreamWidget::handleTrashClicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Delete Dream",
                                  "Are you sure you want to delete this dream?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DreamManager::self()->removeDream(currentDreamID);
        MainWindow::self()->updateWidgets();
        backButton->click();
    }
}

void FullDreamWidget::updateHeart()
{
    heartButton->setShouldModifyFillColor(heartButton->isChecked());
}















