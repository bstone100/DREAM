#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "AI/openai_request.h"
#include "QKeyEvent"
#include "QMenuBar"
#include "QDir"
#include "QComboBox"
#include "QTimer"
#include "QJsonObject"
#include "widgets/sidepanel.h"
#include "QSvgRenderer"
#include "widgets/svgbutton.h"
#include "QButtonGroup"
#include "widgets/resizingtextedit.h"
#include "QGraphicsOpacityEffect"
#include "QParallelAnimationGroup"
#include "QGroupBox"
#include "QStackedLayout"
#include "qstandardpaths.h"
#include "widgets/resizingcombobox.h"
#include "QAudioFormat"
#include "QThread"
#include "audio/audiotranscriptionmanager.h"
#include <QRegularExpression>
#include "widgets/microphonewidget.h"
#include "QLabel"
#include "QJsonDocument"
#include "QScrollBar"
#include "widgets/audiorecorderwidget.h"


#if defined(Q_OS_IOS)
#include "iOS/hapticfeedback.h"
#include "iOS/DarkModeDetector.h"
#elif defined(Q_OS_MACOS)
#include "macOS/MacThemeDetector.h"
#endif


MainWindow *MainWindow::singleton = NULL;
QString MainWindow::version = PROJECT_VERSION;
QString MainWindow::currentPath;

QColor MainWindow::lightColor = 0xE9E9EB;
QColor MainWindow::lightMidColor = 0x2a284c;
QColor MainWindow::darkMidColor = 0x220f30;
QColor MainWindow::darkColor = 0x081f30;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    if (!singleton) {
        singleton = this;
    }

    qApp->setOrganizationName("BenProductions");
    qApp->setApplicationName("Dream Recorder");

    qApp->installEventFilter(this);


// Preprocessor directives to check the platform
#if defined(Q_OS_ANDROID)
    // Use QStandardPaths with AppDataLocation for Android to get a writable location
    currentPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
#elif defined(Q_OS_IOS)
    currentPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
#else
    currentPath = QCoreApplication::applicationDirPath();
#endif


    QDir dir(currentPath);
    if (!dir.exists()) {
        dir.mkpath(currentPath);
    }

    settings = new QSettings;

    centralWidget = new QWidget(this);
    centralWidget->setFocusPolicy(Qt::StrongFocus);
    setCentralWidget(centralWidget);

    layout = new QVBoxLayout(centralWidget);


    themeComboBox = new ResizingComboBox(SidePanel::self());
    QStringList themes = {tr("Light"), tr("Dark"), tr("Auto")};
    themeComboBox->addItems(themes);
    connect(themeComboBox, &QComboBox::currentTextChanged, this, [&]{
        int index = themeComboBox->currentIndex();
        if (index == 0) {
            isAutoTheme = false;
            handleThemeChange(false);
        } else if (index == 1) {
            isAutoTheme = false;
            handleThemeChange(true);
        } else if (index == 2) {
            isAutoTheme = true;
            handleThemeChange(isSystemDark());
        }
        saveSettings();
    });


// fixes mac combo box behavior
#if defined(Q_OS_MACOS)
    themeComboBox->setStyleSheet("combobox-popup: 0;");
#endif


    // hard code this
    apiKey = "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";

    // chat request
    chatRequest = new OpenAIRequest();
    chatRequest->setAccessToken(apiKey);


    QJsonObject systemPrompt;

    systemPrompt["prompt"] = "You are part of an app called Dream Recorder. The app lets the user transcribe a description of the dreams they had last night. "
                             "Your job is to parse the user's dreams and make the correct tool calls."
                             "Only use tools that you have been given access to.";

    chatRequest->addMessage(new OpenAIMessage(systemPrompt, OpenAIMessage::System));

    // side panel
    SvgButton *drawerButton = new SvgButton(this);
    drawerButton->setSvgPath(":/images/drawer.svg");
    drawerButton->setIconSize(QSize(30,30));
    drawerButton->setFixedSize(70, 70);
    drawerButton->setUsingAppColors(true);

    connect(SidePanel::self(), &SidePanel::animationStarted, drawerButton, [=]{
        drawerButton->startColorOverride(drawerButton->appDefaultColor());
    });
    connect(SidePanel::self(), &SidePanel::animationFinished, drawerButton, [=]{
        drawerButton->stopColorOverride();
    });

    connect(drawerButton, &QPushButton::clicked, SidePanel::self(), &SidePanel::toggle);


    // Title for the side panel
    auto titleLabel = new QLabel("Dream Recorder");
    titleLabel->setAlignment(Qt::AlignCenter);
//    titleLabel->setTextFormat(Qt::RichText); // enable HTML tags
    titleLabel->setStyleSheet("QLabel{font-size: 25px;}");

    // Group box for theme settings
    auto appearanceGroupBox = new QGroupBox(tr("Appearance"));
    auto appearanceLayout = new QHBoxLayout;
    auto themeLabel = new QLabel(tr("Theme:"));
    appearanceLayout->addWidget(themeLabel);
    appearanceLayout->addWidget(themeComboBox);
    appearanceGroupBox->setLayout(appearanceLayout);


    QString credits = QString("<p style='line-height: 120%;'>Dream Recorder v%1<br/>© 2024 Benjamin Stone</p>").arg(version);
    auto creditLabel = new QLabel(credits);
    creditLabel->setAlignment(Qt::AlignCenter);
    creditLabel->setStyleSheet("QLabel{font-size: 12px;}");

    // Updating the vertical layout
    auto vLayout = SidePanel::self()->verticalLayout();
    vLayout->setSpacing(20); // Adjust the spacing as needed
    vLayout->addWidget(titleLabel);
//    vLayout->addWidget(appearanceGroupBox);
    vLayout->addStretch();
    vLayout->addWidget(creditLabel);

    // Top layout for dark mode button, voice selection, and record button
    QHBoxLayout *topRowLayout = new QHBoxLayout();
    topRowLayout->addWidget(drawerButton);
    topRowLayout->addStretch();
    topRowLayout->setContentsMargins(0,0,0,0);

    // Adding layouts and widgets to the main layout
    layout->addLayout(topRowLayout);
    layout->addStretch();
    layout->addWidget(AudioRecorderWidget::self());

    auto margins = layout->contentsMargins();
    margins.setTop(0);
    layout->setContentsMargins(margins);

    layout->setAlignment(AudioRecorderWidget::self(), Qt::AlignHCenter);


    scaleBackgroundImage();

    loadSettings();
}


MainWindow::~MainWindow()
{

}

MainWindow *MainWindow::self()
{
    if (!singleton) {
        singleton = new MainWindow();
    }
    return singleton;
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    // Draw the pixmap in the middle of the widget
    qreal ratio = devicePixelRatioF();
    painter.drawPixmap(QRect((width() - scaledBackground.width() / ratio) / 2,
                             (height() - scaledBackground.height() / ratio) / 2,
                             scaledBackground.width() / ratio,
                             scaledBackground.height() / ratio), scaledBackground);
}

void MainWindow::scaleBackgroundImage() {
    QPixmap originalPixmap(":/images/galaxy.png");
    qreal ratio = devicePixelRatioF();
    scaledBackground = originalPixmap.scaled(size().width() * ratio, size().height() * ratio, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    scaledBackground.setDevicePixelRatio(ratio);
}

void MainWindow::updateWidgets()
{
    // call update function on widgets that depend on dream manager
}

void MainWindow::sendChat()
{
    auto transcriptionTextEdit = AudioRecorderWidget::self()->getTranscriptTextEdit();
    if (transcriptionTextEdit->toPlainText() == "") return;

    OpenAIMessage *userMessage = new OpenAIMessage("", OpenAIMessage::Role::User);
    userMessage->setUserMessage(transcriptionTextEdit->toPlainText());
    userMessage->addTimestamp();

    chatRequest->setModel("gpt-4o");

    // TEMP
    return;

    chatRequest->addMessage(userMessage);
    chatRequest->execute();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QMainWindow::closeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_W)
    {
        close();
        return;
    }

    QMainWindow::keyPressEvent(event);
}


bool MainWindow::isSystemDark()
{
#if defined(Q_OS_IOS)
    return isIOSInDarkMode();
#elif defined(Q_OS_MACOS)
    return isMacInDarkMode();
#else
    // TODO: implement for windows and android
    return this->isDarkMode;
#endif
}

void MainWindow::handleThemeChange(bool isDarkMode)
{
    if (!settingsLoaded) return;
    if (this->isDarkMode == isDarkMode) return;

    this->isDarkMode = isDarkMode;
    setDarkMode(isDarkMode);
}


void MainWindow::setDarkMode(bool isDarkMode)
{
    QFile file(":/style/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = file.readAll();

        // this lets us use just one stylesheet and change its colors at runtime
        if (isDarkMode) {
            static QColor sidePanelColorDark = 0x021527;
            static QColor menuBorderColorDark = 0x7A71E7;
            static QColor menuItemSelectedColorDark = 0x5A4EA6;
            static QColor menuItemDisabledColorDark = 0xA095C7;

            styleSheet.replace("@backgroundColor", darkColor.name());
            styleSheet.replace("@foregroundColor", lightColor.name());

            styleSheet.replace("@sidePanelColor", sidePanelColorDark.name());
            styleSheet.replace("@menuBorderColor", menuBorderColorDark.name());
            styleSheet.replace("@menuItemSelectedColor", menuItemSelectedColorDark.name());
            styleSheet.replace("@menuItemDisabledColor", menuItemDisabledColorDark.name());
        } else {
            static QColor sidePanelColorLight = 0xA6A8AF;
            static QColor menuBorderColorLight = 0x5A4EA6;
            static QColor menuItemSelectedColorLight = 0x7A71E7;
            static QColor menuItemDisabledColorLight = 0xB3A6C9;

            styleSheet.replace("@backgroundColor", lightColor.name());
            styleSheet.replace("@foregroundColor", darkColor.name());

            styleSheet.replace("@sidePanelColor", sidePanelColorLight.name());
            styleSheet.replace("@menuBorderColor", menuBorderColorLight.name());
            styleSheet.replace("@menuItemSelectedColor", menuItemSelectedColorLight.name());
            styleSheet.replace("@menuItemDisabledColor", menuItemDisabledColorLight.name());
        }

        qApp->setStyleSheet(styleSheet);
        qApp->processEvents();
    }

    if (isDarkMode) {
        SvgButton::setAppColors(lightColor, lightColor, lightColor, lightColor);
    } else {
        SvgButton::setAppColors(darkColor, darkColor, darkColor, darkColor);
    }
}

void MainWindow::saveSettings()
{
    if (!settingsLoaded) return;

    settings->setValue("apiKey", apiKey);
    settings->setValue("isDarkMode", isDarkMode);
    settings->setValue("isAutoTheme", isAutoTheme);

    settings->setValue("onboarded", onboarded);

    settings->setValue("mainWindow/geometry", saveGeometry());
    settings->setValue("mainWindow/windowState", saveState());
}

void MainWindow::loadSettings()
{
    apiKey = settings->value("apiKey", apiKey).toString();
    isDarkMode = settings->value("isDarkMode", false).toBool();
    isAutoTheme = settings->value("isAutoTheme", true).toBool();

    // this app stays in dark mode
    isDarkMode = true;
    isAutoTheme = false;

    if (isAutoTheme) {
        themeComboBox->setCurrentIndex(2);
    } else if (isDarkMode) {
        themeComboBox->setCurrentIndex(1);
    } else {
        themeComboBox->setCurrentIndex(0);
    }

    if (isAutoTheme) {
        isDarkMode = isSystemDark();
    }
    setDarkMode(isDarkMode);

    onboarded = settings->value("onboarded", false).toBool();
    // do something here if user hasn't been onboarded
    onboarded = true;

    restoreGeometry(settings->value("mainWindow/geometry").toByteArray());
    restoreState(settings->value("mainWindow/windowState").toByteArray());

    QTimer::singleShot(5, this, [=]{
#if defined(Q_OS_IOS)
        // cache the side panel widgets proper geometry
        SidePanel::self()->saveOpenChildWidgetGeometry();
#endif
    });

    settingsLoaded = true;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // handle mobile gestures
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        if (obj->objectName() != "qt_scrollarea_viewport") {
            touchEvent(static_cast<QTouchEvent*>(event));
        }
        break;
    default:
        break;
    }

    // this prevents the whole app from being pushed up when the virtual keyboard comes up
    // but it doesn't move the line edit up
//    if (event->type() == QEvent::InputMethodQuery) {
//        QInputMethodQueryEvent *imEvt = static_cast<QInputMethodQueryEvent *>(event);
//        if (imEvt->queries() == Qt::InputMethodQuery::ImCursorRectangle) {
//            imEvt->setValue(Qt::InputMethodQuery::ImCursorRectangle, QRectF());
//            return true;
//        }
//    }

    // macOS: caught main window change:  QEvent(ThemeChange, 0x16f193498)
    // iOS: caught app change:  QEvent(ApplicationPaletteChange, 0x16b590c00)

    if (isAutoTheme && settingsLoaded) {
#if defined(Q_OS_IOS)
        if (obj == qApp && event->type() == QEvent::ApplicationPaletteChange) {
            handleThemeChange(isSystemDark());
        }
#elif defined(Q_OS_MACOS)
        if (obj == this && event->type() == QEvent::ThemeChange) {
            handleThemeChange(isSystemDark());
        }
#else
        // TODO: test on windows
#endif
    }

    // TODO: use this on windows
    // attempt to catch dark mode light mode change
//    switch (event->type()) {
//    case QEvent::PaletteChange:
//    case QEvent::ApplicationPaletteChange:
//    case QEvent::StyleChange:
//    case QEvent::ThemeChange:
//        if (obj == qApp) {
////            qDebug() << "caught app change: " << event;
//        }
//        if (obj == this) {
////            qDebug() << "caught main window change: " << event;
//        }
//        break;
//    default:
//        break;
//    }

    return QObject::eventFilter(obj, event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    SidePanel::self()->updateSize();
    QMainWindow::resizeEvent(event);
    scaleBackgroundImage();
}


void MainWindow::dumpJsonToFile(QJsonObject &jObj, QString fileName)
{
    // Convert the QJsonObject to QJsonDocument
    QJsonDocument jsonDoc(jObj);

    // Save the QJsonDocument to a file
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonDoc.toJson());
        file.close();
    } else {
        qWarning("Couldn't open file to save JSON data.");
    }
}

QPropertyAnimation *MainWindow::fadeInWidget(QWidget* widget, int duration) {
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
        effect->setOpacity(0); // Start fully transparent if no effect was previously set
    }

    // Create and configure the animation
    QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(duration);
    animation->setStartValue(effect->opacity()); // Start from the current opacity
    animation->setEndValue(1); // Animate to fully opaque
    animation->setEasingCurve(QEasingCurve::InOutQuad); // Smooth transition

    QObject::connect(animation, &QPropertyAnimation::finished, widget, [widget]{
        widget->setEnabled(true);
        if (SvgButton *button = qobject_cast<SvgButton *>(widget)) {
            button->stopColorOverride();
        }
    });

    widget->show(); // Ensure the widget is visible
    animation->start(QPropertyAnimation::DeleteWhenStopped); // Clean up animation when done

    return animation;
}

QPropertyAnimation *MainWindow::fadeOutWidget(QWidget* widget, int duration) {
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
        effect->setOpacity(1); // Assume starting fully opaque if no effect was previously set
    }

    QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(duration);
    animation->setStartValue(effect->opacity()); // Start from the current opacity
    animation->setEndValue(0); // Animate to fully transparent
    animation->setEasingCurve(QEasingCurve::InOutQuad); // Smooth transition

    if (SvgButton *button = qobject_cast<SvgButton *>(widget)) {
        button->startColorOverride(button->activeDefaultColor());
    }
    widget->setEnabled(false);
    animation->start(QPropertyAnimation::DeleteWhenStopped); // Clean up animation when done

    return animation;
}

QPropertyAnimation *MainWindow::fadeInWidgets(QList<QWidget *> widgets, int duration)
{
    QPropertyAnimation *anim = NULL;
    foreach (auto widget, widgets) {
        anim = fadeInWidget(widget, duration);
    }
    return anim;
}

QPropertyAnimation *MainWindow::fadeOutWidgets(QList<QWidget *> widgets, int duration)
{
    QPropertyAnimation *anim = NULL;
    foreach (auto widget, widgets) {
        anim = fadeOutWidget(widget, duration);
    }
    return anim;
}

void MainWindow::setWidgetOpacity(QWidget *widget, double opacity)
{
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
    }
    effect->setOpacity(qBound(0.0, opacity, 1.0));
}

double MainWindow::getWidgetOpacity(QWidget *widget)
{
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    return effect ? effect->opacity() : 1.0;
}

double MainWindow::interpolate(double startVal, double endVal, double progress)
{
    progress = qBound(0.0, progress, 1.0);
    return startVal * (1 - progress) + endVal * progress;
}

void MainWindow::smartSetVisible(QList<QWidget *> widgets, bool visible, int duration, QEasingCurve::Type curveType)
{
    QHash<QWidget *, QList<QWidget *>> parentMap;

    // Group widgets by their parents
    foreach (QWidget *widget, widgets) {
        if (widget && widget->parentWidget()) {
            parentMap[widget->parentWidget()].append(widget);
        }
    }

    if (parentMap.isEmpty()) return;

    if (parentMap.size() > 1) {
        foreach (QList<QWidget *> commonParentList, parentMap) {
            smartSetVisible(commonParentList, visible, duration, curveType);
        }
        return;
    }

    // at this point all widgets have common parent

    // cull widgets whose visibility is already equal to visible
    for (int i = widgets.size() - 1; i >= 0; i--) {
        auto widget = widgets.at(i);
        if (!widget || widget->isVisible() == visible) {
            widgets.removeAt(i);
        }
    }

    if (widgets.isEmpty()) return;

    QWidget *parentWidget = widgets.first()->parentWidget();
    if (!parentWidget) return;

    QLayout *layout = parentWidget->layout();
    if (!layout) return;

    bool isVert = qobject_cast<QVBoxLayout *>(layout);
    bool isHoriz = qobject_cast<QHBoxLayout *>(layout);

    if (!isVert && !isHoriz) return;

    // now it's safe to animate parent

    // save parent geom
    QRect startRect = parentWidget->geometry();
    QRect endRect = startRect;
    QSize oldSize = parentWidget->size();

    // hide or show children
    foreach (auto widget, widgets) {
        widget->setVisible(visible);
    }

    // get target size
    QSize newSize = layout->sizeHint();

    // If there's no size change, skip animation
    if (newSize == oldSize) return;


    if (isVert) {
        // anchor to bottom
        // modify y and height
        int delta = oldSize.height() - newSize.height();
        endRect.setTop(endRect.top() + delta);
    } else {
        // anchor to middle
        // modify x and width
        int delta = oldSize.width() - newSize.width();
        endRect.setLeft(endRect.left() + delta / 2);

        endRect.setWidth(newSize.width());
    }

    QPropertyAnimation *animation = new QPropertyAnimation(parentWidget, "geometry");
    animation->setDuration(duration);
    animation->setEasingCurve(curveType);
    animation->setStartValue(startRect);
    animation->setEndValue(endRect);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}


// determine which gesture is happening and redirect touch events until the finger is lifted
void MainWindow::touchEvent(QTouchEvent *event)
{
    const QList<QTouchEvent::TouchPoint> &touchPoints = event->points();
    if (touchPoints.isEmpty()) return;

    const QTouchEvent::TouchPoint &touchPoint = touchPoints.first();
    QPoint currentTouchPoint = touchPoint.position().toPoint();

    const int edgeThreshold = 30;

    bool onLeftEdge = (currentTouchPoint.x() <= edgeThreshold);

    // set currentGesture based on initial touch
    if (event->type() == QEvent::TouchBegin) {
        if (SidePanel::self()->isVisibleToUser()) {
            // side panel is already showing
            currentGesture = SidePanel;
        } else if (onLeftEdge) {
            // calendar is showing and touch was on left edge
            currentGesture = SidePanel;
        } else {
            currentGesture = Undefined;
        }
    }

    // route event
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        switch (currentGesture) {
        case SidePanel:
            SidePanel::self()->touchEvent(event);
            break;
        case Undefined:
            break;
        }
        break;
    default:
        break;
    }

    // after finger is lifted
    if (event->type() == QEvent::TouchEnd) {
        currentGesture = Undefined;
    }
}






