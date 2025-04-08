#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建地图视图
    m_mapView = new MapView(this);
    setCentralWidget(m_mapView);

    // 创建地图和模拟器
    Map* map = new Map();
    map->generateRandomGraph(1000);

    Simulator* simulator = new Simulator();

    // 设置视图
    m_mapView->setMap(map);
    m_mapView->setSimulator(simulator);
}

MainWindow::~MainWindow()
{
    delete ui;
}
