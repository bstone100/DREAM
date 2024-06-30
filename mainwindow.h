#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QSettings>
#include <QRadioButton>
#include <QTouchEvent>
#include "QTextEdit"
#include "QStackedWidget"
#include "qpropertyanimation.h"
#include "QQueue"
#include "QTableView"
#include "QTimer"
#include "QAudioSink"

class OpenAIRequest;
class AudioRecorder;
class SvgButton;
class ResizingTextEdit;
class AudioLevel;
class ResizingComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow *self();

    void updateWidgets();

    void sendChat();

    void saveSettings();
    void loadSettings();

    static QString version;
    static QString currentPath;

    static QColor lightColor;
    static QColor lightMidColor;
    static QColor darkMidColor;
    static QColor darkColor;

    bool isDarkModeOn(){return isDarkMode;}
    bool isSystemDark();
    void handleThemeChange(bool isDarkMode);

    void dumpJsonToFile(QJsonObject &jObj, QString fileName);
    QPropertyAnimation *fadeInWidget(QWidget *widget, int duration);
    QPropertyAnimation *fadeOutWidget(QWidget *widget, int duration);
    QPropertyAnimation *fadeInWidgets(QList<QWidget *> widgets, int duration);
    QPropertyAnimation *fadeOutWidgets(QList<QWidget *> widgets, int duration);
    void setWidgetOpacity(QWidget *widget, double opacity);
    double getWidgetOpacity(QWidget *widget);
    static double interpolate(double startVal, double endVal, double progress);
    static void smartSetVisible(QList<QWidget *> widgets, bool visible, int duration = 500, QEasingCurve::Type curveType = QEasingCurve::Linear);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    static MainWindow *singleton;

    bool settingsLoaded = false;
    bool onboarded = false;

    void setDarkMode(bool isDarkMode);

    QSettings *settings;

    QWidget *centralWidget;
    QVBoxLayout *layout;

    QPixmap scaledBackground;
    void scaleBackgroundImage();

    ResizingComboBox *themeComboBox;

    QString apiKey;
    bool isDarkMode;
    bool isAutoTheme;

    OpenAIRequest *chatRequest;

    // gestures

    void touchEvent(QTouchEvent *event);

    enum Gesture {
        SidePanel = 0,
        Undefined
    };

    Gesture currentGesture = Undefined;

};

#endif // MAINWINDOW_H








