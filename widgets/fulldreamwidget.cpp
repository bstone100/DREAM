#include "fulldreamwidget.h"
#include "../dreammanager.h"
#include "svgbutton.h"
#include "../mainwindow.h"
#include "../locationmanager.h"
#include "QMessageBox"
#include "resizingtextedit.h"


FullDreamWidget::FullDreamWidget(QWidget *parent) : QWidget(parent)
{
    backButton = new SvgButton(this);
    backButton->setSvgPath(":/images/leftArrow.svg");
    backButton->setIconSize(QSize(30,30));
    backButton->setFixedSize(60, 50);
    backButton->setUsingAppColors(true);

    trashButton = new SvgButton(this);
    trashButton->setSvgPath(":/images/trash.svg");
    trashButton->setIconSize(QSize(30,30));
    trashButton->setFixedSize(60, 50);
    trashButton->setUsingAppColors(true);

    heartButton = new SvgButton(this);
    heartButton->setSvgPath(":/images/heart.svg");
    heartButton->setIconSize(QSize(30,30));
    heartButton->setFixedSize(60, 50);
    heartButton->setUsingAppColors(true);
    heartButton->setCheckable(true);
    heartButton->setChecked(false);

    connect(trashButton, &QPushButton::clicked, this, &FullDreamWidget::handleTrashClicked);

    titleTextEdit = new ResizingTextEdit(this);
    titleTextEdit->setStyleSheet("font-size: 25px;");
    titleTextEdit->setAcceptRichText(false);
    titleTextEdit->setReadOnly(true);
    titleTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    titleTextEdit->setMinHeight(0);
    titleTextEdit->setMaxHeight(100);
    titleTextEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


    dateTimeLocationLabel = new QLabel(this);
    dateTimeLocationLabel->setStyleSheet("font-size: 13px; color: #DBD0B3;");
    //    dateTimeLocationLabel->setWordWrap(true);

    nightmareCheckBox = new QCheckBox("Nightmare", this);
    lucidCheckBox = new QCheckBox("Lucid", this);

    transcriptTextEdit = new ResizingTextEdit(this);
    transcriptTextEdit->setAcceptRichText(false);
    transcriptTextEdit->setReadOnly(true);
    transcriptTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    transcriptTextEdit->setMinHeight(60);
    transcriptTextEdit->setMaxHeight(400);
    transcriptTextEdit->setStyleSheet("font-size: 14px;");


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

    auto vBox = new QVBoxLayout;
    vBox->addWidget(titleTextEdit, 0, Qt::AlignHCenter);
    vBox->addWidget(dateTimeLocationLabel, 0, Qt::AlignHCenter);
    vBox->addLayout(hBox);
    vBox->addWidget(transcriptTextEdit);
    vBox->addStretch();
    vBox->setContentsMargins(16,0,16,0);

    vBox->setAlignment(hBox, Qt::AlignCenter);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topH);
    mainLayout->addLayout(vBox);
    mainLayout->setContentsMargins(0,0,0,0);

    setLayout(mainLayout);

    setAttribute(Qt::WA_StyledBackground);

    connect(backButton, &QPushButton::clicked, MainWindow::self(), &MainWindow::collapseFullDreamWidget);
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

    titleTextEdit->setTextBetter(currentDream.title, Qt::AlignHCenter);

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
        transcriptTextEdit->setTextBetter(currentDream.revisedTranscript);
    } else {
        transcriptTextEdit->setTextBetter(currentDream.originalTranscript);
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
    MainWindow::self()->saveSettings();
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
        MainWindow::self()->saveSettings();
        MainWindow::self()->updateWidgets();
        MainWindow::self()->collapseFullDreamWidget();
    }
}

void FullDreamWidget::updateHeart()
{
    heartButton->setShouldModifyFillColor(heartButton->isChecked());
}















