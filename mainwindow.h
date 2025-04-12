#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "view.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
    // Q_OBJECT 宏：用于支持 Qt 的信号和槽机制。

  public:
    MainWindow(QWidget *parent = nullptr); // parent 参数指定父窗口
    ~MainWindow();

  private:
    Ui::MainWindow *ui;
    MapView *m_mapView; // 指向 MapView 对象的指针，用于显示地图视图
};
#endif // MAINWINDOW_H
