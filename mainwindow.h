#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>
#include <QStandardItemModel>
#include <QPoint>
#include <QMenu>
#include <QStack>
#include <QFile>
#include <QTextCodec>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init_tree_view(QString name, bool flag);       // 初始化文件列表
    QStringList dir_real_all(const QString &dirPath, int cur_index);    // 读取一个目录下的所有文件内容，并以treeview展示
    QString getFile(QModelIndex curIndexe);     // 根据当前的选中行，生成其绝对路径
    void create_note(QFile *file);              // 文件创建
    void save_file(QFile *fileProblem, QFile *fileAnswer);      // 文件保存
    QString read_note(QFile *file);             // 文件读取

private slots:
    void on_action_triggered();             // 创建笔记
    void slotTreeMenu(const QPoint &pos);       // 设置右键菜单
    void slotAdditem();             // 添加子目录
    void slotAddNote(); // 添加问题
    void slotDeleteitem();      // 删除某个文件目录/文件下所有内容
    void on_action_2_triggered();

    void on_actCreateChapter_triggered();

    void on_action_4_triggered();       // 创建大章节

    void on_tView_note_doubleClicked(const QModelIndex &index); // 双击文件时，显示问题内容

    void on_pButton_show_clicked();     // 显示答案内容

    void on_pBtn_save_clicked();        // 保存问题和答案的内容

    void on_action_3_triggered();

    void deleteFileDirectory(const QString &path);
    void on_actionactionDelete_triggered();

    void clear_current_point(QString &delete_name);     // 判断是否需要修改当前的笔记问题指向

    bool is_null(const QString& name);  // 判断输入的内容是否为空，并提示框

private:
    Ui::MainWindow *ui;
    QString noteFile;   // 笔记集路径（一级）
    QString noteName;   // 笔记集名称
    QString chapterName;    // 章节名称
    QString subchapterName; // 子章节名称
    QString questionName;      // 问题名称
    QStandardItemModel *model;  // treeview的数据模型
    QStandardItem* items_1;  // 一级索引
    QStandardItem* items_2;  // 二级索引
    QStandardItem* items_3;  // 三级索引
    QString dir1_name = "problems";
    QString dir2_name = "answers";
    QFile* problem_file;    // 问题文件
    QFile* answer_file;     // 答案文件
    QTextCodec *codec;      // 编码格式，防止中文乱码

};

#endif // MAINWINDOW_H
