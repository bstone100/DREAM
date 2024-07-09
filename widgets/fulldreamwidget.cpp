#include "fulldreamwidget.h"
#include "../dreammanager.h"
#include "QtWidgets/qbuttongroup.h"
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

    transcriptTextEdit = new ResizingTextEdit(this);
    transcriptTextEdit->setAcceptRichText(false);
    transcriptTextEdit->setReadOnly(true);
    transcriptTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    transcriptTextEdit->setMinHeight(60);
    transcriptTextEdit->setMaxHeight(400);
    transcriptTextEdit->setStyleSheet("font-size: 14px; margin: 0px; padding: 0px; border: 0px;");

    dateTimeLocationLabel = new QLabel(this);
    dateTimeLocationLabel->setStyleSheet("font-size: 13px; color: #DBD0B3;");
    //    dateTimeLocationLabel->setWordWrap(true);

    QPushButton *nightmareYesButton = new QPushButton("Yes", this);
    QPushButton *nightmareNoButton = new QPushButton("No", this);
    QPushButton *lucidYesButton = new QPushButton("Yes", this);
    QPushButton *lucidNoButton = new QPushButton("No", this);

    nightmareYesButton->setFixedSize(75, 40);
    nightmareNoButton->setFixedSize(75, 40);
    lucidYesButton->setFixedSize(75, 40);
    lucidNoButton->setFixedSize(75, 40);

    nightmareYesButton->setObjectName("yesNoButton");
    nightmareNoButton->setObjectName("yesNoButton");
    lucidYesButton->setObjectName("yesNoButton");
    lucidNoButton->setObjectName("yesNoButton");

    nightmareYesButton->setCheckable(true);
    nightmareNoButton->setCheckable(true);
    lucidYesButton->setCheckable(true);
    lucidNoButton->setCheckable(true);

    nightmareGroup = new QButtonGroup(this);
    nightmareGroup->addButton(nightmareYesButton, 1); // 1 for "Yes"
    nightmareGroup->addButton(nightmareNoButton, 0);  // 0 for "No"

    lucidGroup = new QButtonGroup(this);
    lucidGroup->addButton(lucidYesButton, 1);
    lucidGroup->addButton(lucidNoButton, 0);

    auto nightmareLabel = new QLabel("Was this a nightmare?", this);
    auto lucidLabel = new QLabel("Was this a lucid dream?", this);

    nightmareLabel->setStyleSheet("font-size: 13px; color: #DBD0B3;");
    lucidLabel->setStyleSheet("font-size: 13px; color: #DBD0B3;");

    auto nightmareLayout = new QHBoxLayout;
    nightmareLayout->addWidget(nightmareLabel);
    nightmareLayout->addStretch();
    nightmareLayout->addWidget(nightmareNoButton);
    nightmareLayout->addWidget(nightmareYesButton);

    auto lucidLayout = new QHBoxLayout;
    lucidLayout->addWidget(lucidLabel);
    lucidLayout->addStretch();
    lucidLayout->addWidget(lucidNoButton);
    lucidLayout->addWidget(lucidYesButton);

    connect(nightmareGroup, &QButtonGroup::buttonClicked, this, &FullDreamWidget::handleWidgetInteraction);
    connect(lucidGroup, &QButtonGroup::buttonClicked, this, &FullDreamWidget::handleWidgetInteraction);
    connect(heartButton, &SvgButton::clicked, this, &FullDreamWidget::handleWidgetInteraction);

    auto topH = new QHBoxLayout;
    topH->addWidget(backButton);
    topH->addStretch();
    topH->addWidget(trashButton);
    topH->addWidget(heartButton);

    auto vBox = new QVBoxLayout;
    vBox->addWidget(titleTextEdit, 0, Qt::AlignHCenter);
    vBox->addWidget(dateTimeLocationLabel, 0, Qt::AlignHCenter);
    vBox->addSpacing(20);
    vBox->addLayout(nightmareLayout);
    vBox->addLayout(lucidLayout);
    vBox->addSpacing(20);
    vBox->addWidget(transcriptTextEdit);
    vBox->addStretch();
    vBox->setContentsMargins(16,0,16,0);


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

    nightmareGroup->button(currentDream.isNightmare ? 1 : 0)->setChecked(true);
    lucidGroup->button(currentDream.isLucid ? 1 : 0)->setChecked(true);

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

    currentDream.isNightmare = nightmareGroup->checkedId() == 1;
    currentDream.isLucid = lucidGroup->checkedId() == 1;

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

        // remove parent dream if this was the only child
        Dream parent = DreamManager::self()->getParentDream(currentDreamID);
        if (parent.childIDs.size() == 1) {
            DreamManager::self()->removeDream(parent.id);
        }

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















