#include "MainWindow.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QClipboard>
#include <QMessageBox>
#include <QFrame>
#include <QScrollBar>
#include <QTimer>
#include <QGroupBox>
#include <QFileDialog>
#include <QThread>
#include <QProgressDialog>

// --- Terminal Dialog 实现 ---
TerminalDialog::TerminalDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("yt-dlp 终端输出");
    resize(800, 500);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QWidget *header = new QWidget(this);
    header->setStyleSheet("background-color: #323233; border-bottom: 1px solid #1e1e1e;");
    header->setFixedHeight(36);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    QLabel *title = new QLabel("yt-dlp.exe — 任务执行中", header);
    title->setStyleSheet("color: #999; font-size: 12px; margin-left: 10px;");
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    layout->addWidget(header);

    logArea = new QTextEdit(this);
    logArea->setReadOnly(true);
    logArea->setStyleSheet("background-color: #0c0c0c; color: #cccccc; border: none; font-family: Consolas, Monospace; font-size: 12px; padding: 10px;");
    layout->addWidget(logArea);

    QWidget *footer = new QWidget(this);
    footer->setStyleSheet("background-color: #323233;");
    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    closeBtn = new QPushButton("关闭", footer);
    closeBtn->hide();
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    footerLayout->addStretch();
    footerLayout->addWidget(closeBtn);
    layout->addWidget(footer);
}

void TerminalDialog::appendLog(const QString &text) {
    logArea->append(text);
    QScrollBar *sb = logArea->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void TerminalDialog::setCompleted(bool completed) {
    if (completed) {
        logArea->append("\n>> 任务完成。");
        closeBtn->show();
    } else {
        closeBtn->hide();
    }

}

void TerminalDialog::clearLogs() {
    logArea->clear();
    closeBtn->hide();
}

// --- Settings Dialog 实现 ---
SettingsDialog::SettingsDialog(QSet<QString> *enabledParams, QMap<QString, QString> *paramValues, QWidget *parent)
    : QDialog(parent), enabledParams(enabledParams), paramValues(paramValues)
{
    setWindowTitle("高级设置");
    resize(600, 500); // 稍微宽一点以容纳输入框
    setupUi();
    if (parentWidget()) setStyleSheet(parentWidget()->styleSheet());
}

void SettingsDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *container = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    QList<Category> categories = AppData::getCategories();
    QList<Parameter> allParams = AppData::getParameters();

    for (const auto &cat : categories) {
        if (!AppData::isSettingsCategory(cat.id)) continue;

        QGroupBox *group = new QGroupBox(cat.title, container);
        QVBoxLayout *groupLayout = new QVBoxLayout(group);
        groupLayout->setSpacing(10);

        for (const auto &param : allParams) {
            if (param.categoryId == cat.id) {
                // 动态生成控件逻辑
                QWidget *row = new QWidget(group);
                QHBoxLayout *rowLayout = new QHBoxLayout(row);
                rowLayout->setContentsMargins(0,0,0,0);

                if (param.type == TypeAction) {
                    // 按钮类型 (如更新)
                    QPushButton *actBtn = new QPushButton(param.description, row); // 按钮文字用 description
                    actBtn->setFixedHeight(32);
                    actBtn->setCursor(Qt::PointingHandCursor);
                    // 如果是更新按钮
                    if (param.id == "gen-update") {
                        actBtn->setText("检查更新 (yt-dlp -U)");
                        connect(actBtn, &QPushButton::clicked, this, &SettingsDialog::onUpdateClicked);
                    }
                    rowLayout->addWidget(actBtn);
                } else {
                    // 复选框
                    QCheckBox *chk = new QCheckBox(param.flag, row);
                    chk->setChecked(enabledParams->contains(param.id));

                    if (param.type == TypeString || param.type == TypePath) {
                        // 输入框
                        QLineEdit *edit = new QLineEdit(row);
                        edit->setPlaceholderText(param.description);
                        if (paramValues->contains(param.id)) {
                            edit->setText(paramValues->value(param.id));
                        } else {
                            edit->setText(param.defaultValue);
                        }
                        edit->setEnabled(chk->isChecked()); // 初始状态

                        connect(chk, &QCheckBox::toggled, this, [this, param, edit](bool checked){
                            if(checked) enabledParams->insert(param.id);
                            else enabledParams->remove(param.id);
                            edit->setEnabled(checked);
                            emit paramsChanged();
                        });

                        connect(edit, &QLineEdit::textChanged, this, [this, param](const QString &text){
                            paramValues->insert(param.id, text);
                            emit paramsChanged();
                        });

                        rowLayout->addWidget(chk);
                        rowLayout->addWidget(edit, 1);
                    } else {
                        // 纯布尔值
                        QLabel *desc = new QLabel(param.description, row);
                        desc->setStyleSheet("color: #858585; margin-left: 10px;");
                        desc->setWordWrap(true);

                        connect(chk, &QCheckBox::toggled, this, [this, param](bool checked){
                            if(checked) enabledParams->insert(param.id);
                            else enabledParams->remove(param.id);
                            emit paramsChanged();
                        });

                        rowLayout->addWidget(chk);
                        rowLayout->addWidget(desc, 1);
                    }
                }
                groupLayout->addWidget(row);
            }
        }
        layout->addWidget(group);
    }

    layout->addStretch();
    scroll->setWidget(container);
    mainLayout->addWidget(scroll);

    // 底部
    QWidget *bottom = new QWidget(this);
    bottom->setStyleSheet("background-color: #252526; border-top: 1px solid #3e3e42;");
    QHBoxLayout *btnLayout = new QHBoxLayout(bottom);
    QPushButton *okBtn = new QPushButton("关闭", bottom);
    okBtn->setFixedWidth(100);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    mainLayout->addWidget(bottom);
}

void SettingsDialog::onUpdateClicked() {
    // 检查更新逻辑
    QProgressDialog progress("正在检查 yt-dlp 更新...", "取消", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0); // 立即显示
    progress.setValue(0); // Busy indicator

    QProcess updateProc;
    updateProc.setProgram("yt-dlp");
    updateProc.setArguments(QStringList() << "-U");

    connect(&updateProc, &QProcess::finished, [&](int exitCode, QProcess::ExitStatus){
        progress.close();
        QString output = QString::fromLocal8Bit(updateProc.readAllStandardOutput());
        if (output.isEmpty()) output = QString::fromLocal8Bit(updateProc.readAllStandardError());

        QMessageBox::information(this, "更新结果", output.isEmpty() ? "检查完成 (无输出)" : output);
    });

    updateProc.start();

    if (!updateProc.waitForStarted(3000)) {
        progress.close();
        QMessageBox::critical(this, "错误", "无法启动 yt-dlp，请检查是否安装。");
        return;
    }

    // 保持 UI 响应
    while (updateProc.state() == QProcess::Running) {
        QApplication::processEvents();
        if (progress.wasCanceled()) {
            updateProc.kill();
            break;
        }
        QThread::msleep(50);
    }
}


// --- MainWindow 实现 ---

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), process(new QProcess(this)), terminal(new TerminalDialog(this))
{
    setupUi();
    setupStyle();

    // 默认值初始化
    enabledParams.insert("fmt-best");

    // 初始化有默认值的 String/Path 参数值
    for(const auto &p : AppData::getParameters()) {
        if((p.type == TypeString || p.type == TypePath) && !p.defaultValue.isEmpty()) {
            paramValues.insert(p.id, p.defaultValue);
        }
    }

    loadMainParams();
    updateCommandPreview();

    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::onProcessOutput);
    connect(process, &QProcess::readyReadStandardError, this, &MainWindow::onProcessOutput);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::onProcessFinished);
    connect(process, &QProcess::errorOccurred, this, &MainWindow::onProcessError);
}

MainWindow::~MainWindow() {
    if (process->state() == QProcess::Running) {
        process->kill();
        process->waitForFinished();
    }
}

void MainWindow::setupUi() {
    resize(1100, 750);
    setWindowTitle("yt-dlp GUI (Qt6)");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 顶部栏 ---
    QWidget *topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(70);
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);

    QVBoxLayout *urlContainer = new QVBoxLayout();
    QLabel *urlLabel = new QLabel("视频链接 (URL):", topBar);
    urlLabel->setStyleSheet("color: #858585; font-weight: bold; text-transform: uppercase; font-size: 10px;");

    QHBoxLayout *urlInputRow = new QHBoxLayout();
    urlInput = new QLineEdit(topBar);
    urlInput->setPlaceholderText("https://www.youtube.com/watch?v=...");
    connect(urlInput, &QLineEdit::textChanged, this, &MainWindow::updateCommandPreview);

    QPushButton *clearBtn = new QPushButton("清空", topBar);
    clearBtn->setFixedWidth(60);
    connect(clearBtn, &QPushButton::clicked, [this](){ urlInput->clear(); });

    urlInputRow->addWidget(urlInput);
    urlInputRow->addWidget(clearBtn);
    urlContainer->addWidget(urlLabel);
    urlContainer->addLayout(urlInputRow);
    urlContainer->setAlignment(Qt::AlignVCenter);

    settingsBtn = new QPushButton("⚙ 设置", topBar);
    settingsBtn->setFixedWidth(80);
    settingsBtn->setFixedHeight(32);
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::openSettings);

    topLayout->addLayout(urlContainer, 1);
    topLayout->addSpacing(20);
    topLayout->addWidget(settingsBtn);
    mainLayout->addWidget(topBar);

    // --- 内容区 ---
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    contentAreaWidget = new QWidget();
    contentLayout = new QVBoxLayout(contentAreaWidget);
    contentLayout->setAlignment(Qt::AlignTop);
    contentLayout->setContentsMargins(30, 20, 30, 20);
    contentLayout->setSpacing(25);

    scrollArea->setWidget(contentAreaWidget);
    mainLayout->addWidget(scrollArea, 1);

    // --- 底部栏 ---
    QWidget *bottomBar = new QWidget(this);
    bottomBar->setObjectName("bottomBar");
    bottomBar->setMinimumHeight(100);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);

    QWidget *previewBox = new QWidget(this);
    previewBox->setStyleSheet("background-color: #2d2d2d; border-top: 2px solid #007acc; padding: 10px;");
    QVBoxLayout *prevLayout = new QVBoxLayout(previewBox);
    prevLayout->setContentsMargins(10, 5, 10, 5);

    QLabel *prevTitle = new QLabel("生成的命令预览", previewBox);
    prevTitle->setStyleSheet("color: #858585; font-weight: bold; font-size: 10px; text-transform: uppercase; margin-bottom: 5px;");

    commandPreviewLabel = new QLabel(previewBox);
    commandPreviewLabel->setStyleSheet("color: #ce9178; font-family: Consolas, Monospace; font-size: 12px;");
    commandPreviewLabel->setWordWrap(true);
    commandPreviewLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    prevLayout->addWidget(prevTitle);
    prevLayout->addWidget(commandPreviewLabel, 1);

    bottomLayout->addWidget(previewBox, 1);

    copyBtn = new QPushButton("复制", bottomBar);
    copyBtn->setObjectName("actionBtn");
    copyBtn->setFixedWidth(100);
    connect(copyBtn, &QPushButton::clicked, this, &MainWindow::copyCommand);

    runBtn = new QPushButton("运行", bottomBar);
    runBtn->setObjectName("runBtn");
    runBtn->setFixedWidth(120);
    connect(runBtn, &QPushButton::clicked, this, &MainWindow::runCommand);

    bottomLayout->addWidget(copyBtn);
    bottomLayout->addWidget(runBtn);
    mainLayout->addWidget(bottomBar);
}

void MainWindow::setupStyle() {
    QString qss = R"(
        QMainWindow, QDialog { background-color: #1e1e1e; }
        QWidget { color: #cccccc; font-family: "Segoe UI", sans-serif; font-size: 13px; }

        QWidget#topBar { background-color: #252526; border-bottom: 1px solid #3e3e42; }

        QLineEdit {
            background-color: #3c3c3c;
            border: 1px solid #3e3e42;
            color: white;
            padding: 5px 10px;
            border-radius: 4px;
        }
        QLineEdit:focus { border: 1px solid #007acc; }
        QLineEdit:disabled { background-color: #2a2a2a; color: #666; border: 1px solid #333; }

        QGroupBox {
            border: 1px solid #3e3e42;
            border-radius: 6px;
            margin-top: 24px;
            padding-top: 15px;
            background-color: #252526;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 15px;
            padding: 0 5px;
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
        }

        QScrollArea { background-color: #1e1e1e; border: none; }

        QCheckBox { spacing: 8px; padding: 2px; font-family: Consolas, Monospace; font-weight: bold; color: #dcdcdc; }
        QCheckBox::indicator { width: 18px; height: 18px; border-radius: 3px; border: 1px solid #555; background: #1e1e1e; }
        QCheckBox::indicator:hover { border-color: #888; }
        QCheckBox::indicator:checked { background-color: #007acc; border-color: #007acc; }

        QPushButton {
            background-color: #3c3c3c;
            border: 1px solid #3e3e42;
            color: white;
            border-radius: 4px;
            padding: 5px;
        }
        QPushButton:hover { background-color: #4a4a4a; }
        QPushButton:pressed { background-color: #2d2d2d; }

        QPushButton#actionBtn { background-color: #3e3e42; border: none; border-left: 1px solid #2d2d2d; border-radius: 0; height: 100%; }
        QPushButton#runBtn { background-color: #007acc; border: none; border-radius: 0; font-weight: bold; height: 100%; }
        QPushButton#runBtn:hover { background-color: #0062a3; }

        QScrollBar:vertical { border: none; background: #2b2b2b; width: 10px; margin: 0; }
        QScrollBar::handle:vertical { background: #4a4a4a; min-height: 20px; border-radius: 5px; }
        QScrollBar::handle:vertical:hover { background: #5a5a5a; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )";
    this->setStyleSheet(qss);
}

// 辅助：生成参数控件
QWidget* MainWindow::createParamWidget(const Parameter &param) {
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QCheckBox *chk = new QCheckBox(param.flag, widget);
    chk->setMinimumWidth(180);
    chk->setChecked(enabledParams.contains(param.id));

    layout->addWidget(chk);

    if (param.type == TypeString || param.type == TypePath) {
        QLineEdit *edit = new QLineEdit(widget);
        edit->setPlaceholderText(param.description);
        if (paramValues.contains(param.id)) {
            edit->setText(paramValues[param.id]);
        } else {
            edit->setText(param.defaultValue);
        }
        edit->setEnabled(chk->isChecked());

        connect(edit, &QLineEdit::textChanged, this, [this, param](const QString &text){
            paramValues.insert(param.id, text);
            updateCommandPreview();
        });

        connect(chk, &QCheckBox::toggled, this, [this, param, edit](bool checked){
            if(checked) enabledParams.insert(param.id);
            else enabledParams.remove(param.id);
            edit->setEnabled(checked);
            updateCommandPreview();
        });

        layout->addWidget(edit, 1);

        if (param.type == TypePath) {
            QPushButton *browseBtn = new QPushButton("...", widget);
            browseBtn->setFixedWidth(30);
            connect(browseBtn, &QPushButton::clicked, this, [this, edit](){
                QString dir = QFileDialog::getExistingDirectory(this, "选择下载目录");
                if (!dir.isEmpty()) {
                    edit->setText(dir);
                }
            });
            layout->addWidget(browseBtn);
        }
    } else {
        // Boolean
        QLabel *desc = new QLabel(param.description, widget);
        desc->setStyleSheet("color: #999;");
        desc->setWordWrap(true);
        layout->addWidget(desc, 1);

        connect(chk, &QCheckBox::toggled, this, [this, param](bool checked){
            if(checked) enabledParams.insert(param.id);
            else enabledParams.remove(param.id);
            updateCommandPreview();
        });
    }

    return widget;
}

void MainWindow::loadMainParams() {
    QList<Category> categories = AppData::getCategories();
    QList<Parameter> allParams = AppData::getParameters();

    for (const auto &cat : categories) {
        if (AppData::isSettingsCategory(cat.id)) continue;

        QGroupBox *group = new QGroupBox(cat.title, contentAreaWidget);
        QVBoxLayout *groupLayout = new QVBoxLayout(group);
        groupLayout->setSpacing(8);
        groupLayout->setContentsMargins(15, 15, 15, 15);

        for (const auto &param : allParams) {
            if (param.categoryId == cat.id) {
                groupLayout->addWidget(createParamWidget(param));
            }
        }
        contentLayout->addWidget(group);
    }
    contentLayout->addStretch();
}

void MainWindow::openSettings() {
    SettingsDialog dlg(&enabledParams, &paramValues, this);
    connect(&dlg, &SettingsDialog::paramsChanged, this, &MainWindow::updateCommandPreview);
    dlg.exec();
    updateCommandPreview();
}

void MainWindow::updateCommandPreview() {
    QString cmd = "yt-dlp";

    QList<Parameter> allParams = AppData::getParameters();
    for (const auto &p : allParams) {
        if (enabledParams.contains(p.id)) {
            if (p.type == TypeBoolean) {
                cmd += " " + p.flag;
            } else if (p.type == TypeString || p.type == TypePath) {
                QString val = paramValues.value(p.id);
                if (val.isEmpty()) val = p.defaultValue;
                // 简单转义处理：如果包含空格且没引号，加引号
                if (val.contains(" ") && !val.startsWith("\"")) {
                    val = "\"" + val + "\"";
                }
                cmd += " " + p.flag + " " + val;
            }
        }
    }

    QString url = urlInput->text().trimmed();
    if (url.isEmpty()) cmd += " \"[链接]\"";
    else cmd += " \"" + url + "\"";

    commandPreviewLabel->setText(cmd);
}

void MainWindow::copyCommand() {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(commandPreviewLabel->text());
    copyBtn->setText("已复制!");
    QTimer::singleShot(2000, [this](){ copyBtn->setText("复制"); });
}

void MainWindow::runCommand() {
    // 1. 如果正在运行，点击按钮则终止进程
    if (process->state() != QProcess::NotRunning) {
        process->kill();
        terminal->appendLog("\n>> [操作] 用户中止任务...");
        return;
    }

    QString url = urlInput->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入视频链接 (URL)！");
        return;
    }

    QString program = "yt-dlp";
    QStringList args;

    QList<Parameter> allParams = AppData::getParameters();
    for (const auto &p : allParams) {
        if (enabledParams.contains(p.id)) {
            if (p.type == TypeBoolean) {
                QString raw = p.flag;
                int sp = raw.indexOf(' ');
                if (sp > 0) {
                    args << raw.left(sp);
                    QString rest = raw.mid(sp+1);
                    if(rest.startsWith('"')) rest = rest.mid(1, rest.length()-2);
                    args << rest;
                } else {
                    args << raw;
                }
            } else if (p.type == TypeString || p.type == TypePath) {
                args << p.flag;
                QString val = paramValues.value(p.id);
                if(val.isEmpty()) val = p.defaultValue;
                args << val;
            }
        }
    }

    args << url;

    terminal->clearLogs();
    terminal->show();
    terminal->appendLog(QString("> 执行: %1 %2").arg(program, args.join(" ")));

    process->setProgram(program);
    process->setArguments(args);

    // 2. 更改按钮状态为“停止”
    runBtn->setText("停止");
    runBtn->setStyleSheet("background-color: #d71526; color: white; font-weight: bold;");
    runBtn->setEnabled(true); // 保持激活，允许点击停止
    process->start();
}

void MainWindow::onProcessOutput() {
    QByteArray data = process->readAllStandardOutput();
    QByteArray errData = process->readAllStandardError();
    if (!data.isEmpty()) terminal->appendLog(QString::fromLocal8Bit(data));
    if (!errData.isEmpty()) terminal->appendLog(QString::fromLocal8Bit(errData));
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    // 恢复按钮状态
    runBtn->setText("运行");
    runBtn->setStyleSheet(""); // 恢复默认样式表
    runBtn->setEnabled(true);

    QString status = (exitStatus == QProcess::NormalExit && exitCode == 0) ? "成功" : "失败/中断";
    terminal->appendLog(QString("\n[进程结束] 状态: %1 (代码: %2)").arg(status).arg(exitCode));
    runBtn->setText("运行");
    runBtn->setStyleSheet("");
    runBtn->setEnabled(true);
    terminal->setCompleted(true);
}

void MainWindow::onProcessError(QProcess::ProcessError error) {
    // 处理所有错误情况
    QString errStr;
    switch(error) {
    case QProcess::FailedToStart: errStr = "无法启动程序 (找不到 yt-dlp)"; break;
    case QProcess::Crashed: errStr = "进程崩溃"; break;
    case QProcess::Timedout: errStr = "操作超时"; break;
    case QProcess::WriteError: errStr = "写入错误"; break;
    case QProcess::ReadError: errStr = "读取错误"; break;
    default: errStr = "未知错误"; break;
    }

    terminal->appendLog("\n[系统错误] " + errStr);

    // 只有在 FailedToStart 时，finished 信号可能不会触发，必须手动重置
    // 其他错误通常会伴随 finished 信号，但为了保险起见，如果进程不在运行中，我们强制重置
    // if (error == QProcess::FailedToStart || process->state() == QProcess::NotRunning) {
        runBtn->setText("运行");
        runBtn->setStyleSheet("");
        runBtn->setEnabled(true);
        terminal->setCompleted(true);
    // }
}
