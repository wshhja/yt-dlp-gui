#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QDialog>
#include <QSet>
#include <QMap>
#include <QPushButton>
#include "ParamData.h"

// 终端对话框
class TerminalDialog : public QDialog {
    Q_OBJECT
public:
    explicit TerminalDialog(QWidget *parent = nullptr);
    void appendLog(const QString &text);
    void setCompleted(bool completed);
    void clearLogs();

private:
    QTextEdit *logArea;
    QPushButton *closeBtn;
};

// 设置对话框
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    // 传递参数状态指针
    explicit SettingsDialog(QSet<QString> *enabledParams, QMap<QString, QString> *paramValues, QWidget *parent = nullptr);

signals:
    void paramsChanged();

private slots:
    void onUpdateClicked();

private:
    void setupUi();
    QSet<QString> *enabledParams;
    QMap<QString, QString> *paramValues;
};

// 主窗口
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openSettings();
    void updateCommandPreview();
    void copyCommand();
    void runCommand();

    // 进程相关
    void onProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    void setupUi();
    void setupStyle();
    void loadMainParams();

    // 辅助函数：创建不同类型的参数控件
    QWidget* createParamWidget(const Parameter &param);

    // UI 组件
    QWidget *contentAreaWidget;
    QVBoxLayout *contentLayout;
    QLineEdit *urlInput;
    QLabel *commandPreviewLabel;
    QPushButton *runBtn;
    QPushButton *copyBtn;
    QPushButton *settingsBtn;

    // 数据模型
    QSet<QString> enabledParams;        // 存储已启用的参数ID
    QMap<QString, QString> paramValues; // 存储参数的输入值 (ID -> Value)

    QProcess *process;
    TerminalDialog *terminal;
};

#endif // MAINWINDOW_H
