#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_tree_view(QString name, bool flag)
{
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList() << name);
    // 如果是以打开已有笔记集的方式，那么需要读取下面所有文件
    if (!flag) {
       dir_real_all(noteFile, 0);
    }
     ui->tView_note->setModel(model);
     ui->tView_note->setEditTriggers(QTreeView::NoEditTriggers);
    // 创建右键菜单功能
    ui->tView_note->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tView_note, &QTreeView::customContextMenuRequested, this, &MainWindow::slotTreeMenu);
    codec=QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
}

QStringList MainWindow::dir_real_all(const QString &dirPath, int cur_index)
{
    QStringList fileList;
    QDir dir(dirPath);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);
    // 递归读取
    foreach (auto fileInfo, fileInfoList)
    {
        QString dir_name = fileInfo.absoluteFilePath().split('/').last();  // 从绝对路径中获取文件夹名称
        // 判断是否为文件夹类型，且名称如果是answers跳过
        if(fileInfo.isDir() && dir_name != "answers")
        {
            // 判断选择的是几级目录，这里简化写法

            if (cur_index == 0) {   // 一级目录，大章节
                items_1 = new QStandardItem(dir_name);
                model->appendRow(items_1);
            } else if (cur_index == 1){    // 二级目录，表示子章节
                items_2 = new QStandardItem(dir_name);
                items_1->appendRow(items_2);
            }
            // 继续向内搜索
            dir_real_all(fileInfo.absoluteFilePath(), cur_index+1);
        }

        if (fileInfo.isFile())
        {
            items_3 = new QStandardItem(dir_name);
            items_2->appendRow(items_3);
        }

    }
    return fileList;
}

QString MainWindow::getFile(QModelIndex curIndex)
{
    QStack<QString> names;
    QString filePath(noteFile);
    // 依次将名称添加
    while(curIndex.isValid()) {
        names.push(curIndex.data().toString());
        curIndex = curIndex.parent();
    }
    while(!names.isEmpty()) {
        filePath.append('/'+names.top());
        names.pop();
    }
    return filePath;
}

void MainWindow::create_note(QFile *file)
{
    if (!file->open(QIODevice::Append | QIODevice::Text))
    {
        QMessageBox::critical(this,"错误","文件打开失败，信息没有保存！","确定");
        return;
    }
    file->close();
}

void MainWindow::save_file(QFile *fileProblem, QFile *fileAnswer)
{
    if (fileProblem == NULL) {
        QMessageBox::critical(this,"错误","文件保存失败，请选中文件！","确定");
        return;
    }

    if (ui->txtAnswer->toPlainText().isEmpty())
    {
        QMessageBox::StandardButton box;
        box = QMessageBox::question(this,"提示","答案为空，确定保存吗？", QMessageBox::Yes | QMessageBox::No);
        if (box == QMessageBox::No) {
            return;
        }
    }

    if (fileProblem->open(QIODevice::WriteOnly | QIODevice::Text) && fileAnswer->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out_problem(fileProblem);
        QTextStream out_answer(fileAnswer);
        QString problem_content = ui->txtProblem->toPlainText();
        QString answer_content = ui->txtAnswer->toPlainText();
        out_problem << problem_content;
        out_answer << answer_content;
        fileProblem->close();
        fileAnswer->close();
        QMessageBox::information(this, "笔记保存对话框", "笔记修改成功", QMessageBox::Ok, QMessageBox::Ok);
    }
}

QString MainWindow::read_note(QFile *file)
{
    if(file == NULL) {
        QMessageBox::critical(this,"错误","文件打开失败，请选中文件！","确定");
        return "";
    }
    QString content;
    if (file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(file);
        content = in.readAll();
    }
    file->close();
    return content;
}

// 用于创建笔记集（确定路径）
void MainWindow::on_action_triggered()
{
    ui->txtAnswer->clear();
    ui->txtProblem->clear();
//    qDebug() << "this is triggered!";
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setFileMode(QFileDialog::Directory);    // 设置后可选择目录
    fileDialog->setDirectory("E:/Users");
    fileDialog->setViewMode(QFileDialog::Detail);
    // 选择创建路径并设置名称
    if (fileDialog->exec()) {
        noteFile = fileDialog->selectedFiles()[0];  // 获得笔记的基路径
        bool ok = false;
        noteName = QInputDialog::getText(this, "笔记集创建", "请输入笔记集名称", QLineEdit::Normal, "", &ok);
        bool isNull = is_null(noteName);
        if (ok && !isNull) {
            qDebug() << "当前路径：" << noteFile << endl;
            QDir dir(noteFile);
            // 判断是否重名
            if (!dir.exists(noteName))
            {
                dir.mkdir(noteName);

                QMessageBox::information(this, "笔记集创建对话框", "笔记集创建成功", QMessageBox::Ok, QMessageBox::Ok);
                init_tree_view(noteName, true);
            } else {
                QMessageBox::critical(this, "笔记集创建对话框", "笔记集创建失败", QMessageBox::Ok, QMessageBox::Ok);
            }
        }

    }

}

void MainWindow::slotTreeMenu(const QPoint &pos)
{
    QMenu menu;

    QModelIndex curIndex = ui->tView_note->indexAt(pos);    // 获取当前选中的行索引
    if (curIndex.isValid()) {
        // 不同层级的item需要创建不同右键菜单
        if (curIndex.parent() == ui->tView_note->rootIndex()) {
            menu.addAction(QStringLiteral("添加子章节"), this, &MainWindow::slotAdditem);
        }
        if (curIndex.parent().isValid() && curIndex.parent().parent() == ui->tView_note->rootIndex())
        {
            menu.addAction(QStringLiteral("添加新问题"), this, &MainWindow::slotAddNote);
        }
        menu.addAction(QStringLiteral("删除"), this, &MainWindow::slotDeleteitem);

    }

    menu.exec(QCursor::pos());

}

// 在菜单设置处被链接
void MainWindow::slotAdditem()
{

    QModelIndex curIndex = ui->tView_note->currentIndex();
    QString parentName = curIndex.data().toString();
    QString chapterFile = noteFile + "/" + parentName;
    qDebug() << "当前路径：" << chapterFile << endl;
    bool ok = false;
    QDir dir(chapterFile);
    QString chapterName = QInputDialog::getText(this, "子章节创建", "请输入子章节名称", QLineEdit::Normal, "", &ok);
    bool isNull = is_null(chapterName);
    if(ok && !dir.exists(chapterName) && !isNull) {
        dir.mkdir(chapterName);
        QDir sub_dir(chapterFile+'/'+chapterName);
        sub_dir.mkdir("problems");
        sub_dir.mkdir("answers");
        QMessageBox::information(this, "子章节创建对话框", "子章节创建成功", QMessageBox::Ok, QMessageBox::Ok);
        items_2 = new QStandardItem(chapterName);
        items_1 = model->itemFromIndex(curIndex);
        items_1->appendRow(items_2);
    } else {
        QMessageBox::critical(this, "子章节创建对话框", "子章节创建失败", QMessageBox::Ok, QMessageBox::Ok);
    }
}
// 在菜单设置处被链接
void MainWindow::slotAddNote()
{
    QModelIndex curIndex = ui->tView_note->currentIndex();  // 获取当前选中行
    // 生成路径
    QString chapterFile = getFile(curIndex);
    bool ok = false;
    QDir dir(chapterFile + '/' + dir1_name);
    QString chapterName = QInputDialog::getText(this, "笔记问题创建", "请输入笔记问题名称", QLineEdit::Normal, "", &ok);
    bool isNull = is_null(chapterName);
    if(ok && !dir.exists(chapterName) && !isNull) {
        problem_file = new QFile(chapterFile+'/'+dir1_name+'/'+chapterName+".txt");
        answer_file = new QFile(chapterFile+'/'+dir2_name+'/'+chapterName+".txt");
        qDebug() << "文件名称" << chapterFile+'/'+dir2_name+'/'+chapterName+".txt" << endl;
        create_note(problem_file);
        create_note(answer_file);
        QMessageBox::information(this, "笔记问题创建对话框", "笔记问题创建成功", QMessageBox::Ok, QMessageBox::Ok);
        items_2 = new QStandardItem(chapterName+".txt");
        items_1 = model->itemFromIndex(curIndex);
        items_1->appendRow(items_2);
        ui->txtAnswer->setText("");
        ui->txtProblem->setText("");
        ui->labelStatus->setText("当前题目：" + chapterName);
    } else {
        QMessageBox::critical(this, "笔记问题创建对话框", "笔记问题创建失败", QMessageBox::Ok, QMessageBox::Ok);
    }
}

void MainWindow::slotDeleteitem()
{
    QModelIndex curIndex = ui->tView_note->currentIndex();  // 获得选中行
    QString fileName =  curIndex.data().toString();
    clear_current_point(fileName);
    QString filePath;
    // 由于问题集与上一级目录有隔了一层隐含，所以需要区别划分
    if (fileName.endsWith(".txt")) {
        fileName = getFile(curIndex.parent());
        filePath = fileName+'/'+dir1_name+'/'+curIndex.data().toString();
        deleteFileDirectory(filePath);
        filePath = fileName+'/'+dir2_name+'/'+curIndex.data().toString();
        deleteFileDirectory(filePath);

    }
    else {
        filePath = getFile(curIndex);
        deleteFileDirectory(filePath);
    }
    qDebug() << "delete curIndex:" << curIndex.row() << curIndex.column() << curIndex.parent();
    model->removeRow(curIndex.row(), curIndex.parent());
    ui->tView_note->setModel(model);


}

void MainWindow::on_action_2_triggered()
{
    ui->txtAnswer->clear();
    ui->txtProblem->clear();
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setFileMode(QFileDialog::Directory);
    fileDialog->setDirectory("E:/Users");
    fileDialog->setViewMode(QFileDialog::Detail);
    if (fileDialog->exec()) {
        noteFile = fileDialog->selectedFiles()[0];
        noteName = noteFile.split('/').last();
        init_tree_view(noteName, false);
    }


}

void MainWindow::on_actCreateChapter_triggered()
{

}

void MainWindow::on_action_4_triggered()
{
    bool ok = false;
    QDir dir(noteFile);

    QString chapterName = QInputDialog::getText(this, "章节创建", "请输入新章节名称", QLineEdit::Normal, "", &ok);
    bool isNull = is_null(chapterName);
    if(ok && !dir.exists(chapterName) && !isNull) {
        dir.mkdir(chapterName);
        QMessageBox::information(this, "章节创建对话框", "章节创建成功", QMessageBox::Ok, QMessageBox::Ok);
        items_1 = new QStandardItem(chapterName);
        model->appendRow(items_1);
    } else {
        QMessageBox::critical(this, "章节创建对话框", "章节创建失败", QMessageBox::Ok, QMessageBox::Ok);
    }
}

void MainWindow::on_tView_note_doubleClicked(const QModelIndex &index)
{
    QString fileName = index.data().toString();
    if(fileName.endsWith(".txt")) {
        QString chapterFile = getFile(index.parent());
        problem_file = new QFile(chapterFile+'/'+dir1_name+'/'+fileName);
        answer_file = new QFile(chapterFile+'/'+dir2_name+'/'+fileName);
        questionName = index.data().toString();
        qDebug() << "current answerName:" << chapterFile+'/'+dir2_name+'/'+fileName << endl;
        ui->txtProblem->setText(read_note(problem_file));
        ui->txtAnswer->clear();
        ui->labelStatus->setText("当前题目：" + questionName.split(".txt")[0]);
    }
}

void MainWindow::on_pButton_show_clicked()
{
    ui->txtAnswer->setText(read_note(answer_file));

}

void MainWindow::on_pBtn_save_clicked()
{
    save_file(problem_file, answer_file);
}

void MainWindow::on_action_3_triggered()
{
    save_file(problem_file, answer_file);
}

void MainWindow::deleteFileDirectory(const QString &path)
{
    QFileInfo fileInfo(path);
    // 如果是文件夹就全删，否则只删除自己
    if (fileInfo.exists()) {
        if (fileInfo.isDir()) {
            QDir dir(path);
            dir.removeRecursively();
        }
        else {
            QFile file(path);
            file.remove();
        }
    }
}


void MainWindow::on_actionactionDelete_triggered()
{
    slotDeleteitem();
}

void MainWindow::clear_current_point(QString &delete_name)
{
    if(delete_name.endsWith(".txt")) {
        if (delete_name == questionName) {
            answer_file = NULL;
            problem_file = NULL;
            questionName = "";
            ui->labelStatus->setText("当前题目：");
        }
    }
    else {
        if (delete_name == chapterName) {
            answer_file = NULL;
            problem_file = NULL;
            questionName = "";
            chapterName = "";
            ui->labelStatus->setText("当前题目：");
        }
    }
}

bool MainWindow::is_null(const QString &name)
{
    if (name.isEmpty()) {
        QMessageBox::critical(this, "错误提示框", "您输入的内容为空！", QMessageBox::Ok, QMessageBox::Ok);
    }
    return name.isEmpty();
}
