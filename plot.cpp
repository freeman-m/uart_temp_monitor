#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QDateTime>

#include <QMessageBox>
#include <QDebug>

#include <QTimerEvent>

#include "qcustomplot.h"

#include "plot.h"

uint32_t x_cnt;
int pause_flag=0;

void MainWindow::plot_init(void)
{
    ui->customPlot->xAxis->setRange(D_MY_PLOT_X_MIN, D_MY_PLOT_X_MAX);
    ui->customPlot->yAxis->setRange(D_MY_PLOT_Y_MIN, D_MY_PLOT_Y_MAX);

    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(true);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(true);

    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(Qt::red));
    ui->customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssDisc,2));

    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(Qt::green));
    ui->customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssDisc,2));

    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setPen(QPen(Qt::blue));
    ui->customPlot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssDisc,2));

//    ui->customPlot->graph(0)->setData(x, y);

//    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

#if 0
    // 生成数据，画出的是抛物线
    QVector<double> x(11), y(11),y1(11); //初始化向量x和y
    for (int i=0; i<11; ++i)
    {
      x[i] = i/5.0 - 1; // x范围[-1,1]
      y[i] = x[i]*x[i]; // y=x*x
      y1[i] = y[i]+1;
    }

    // 设定背景为黑色
    ui->customPlot->setBackground(QBrush(Qt::white));
    // 设定右上角图形标注可见
    ui->customPlot->legend->setVisible(true);
    // 设置右上角图形标注的字体
    ui->customPlot->legend->setFont(QFont("Helvetica", 9));

    // 设置坐标轴的范围，以看到所有数据
    ui->customPlot->xAxis->setRange(-2, 2);
    ui->customPlot->yAxis->setRange(0, 1);

    // 一个轴矩形包含4个轴，上下左右，设置可以显示的坐标轴 (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(true);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(true);

    // 为4个坐标轴添加标签文本
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    ui->customPlot->xAxis2->setLabel("x2");
    ui->customPlot->yAxis2->setLabel("y2");
    ui->customPlot->xAxis->setLabelFont(QFont("Helvetica", 26));


    // 添加数据曲线（一个图像可以有多个数据曲线）
    ui->customPlot->addGraph();

    // 设置画笔 需要放在曲线之后
    ui->customPlot->graph(0)->setPen(QPen(Qt::black));
//    ui->customPlot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsNone);
    ui->customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssDisc,5));



    ui->customPlot->graph(0)->setData(x, y);
//    ui->customPlot->graph(0)->setName("第一个示例");// 设置图例名称

    // 添加第二个数据曲线
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setData(x, y1);

    // 自动跳帧坐标轴 make left and bottom axes always transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    //  自适应坐标轴
    // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
    ui->customPlot->graph(0)->rescaleAxes();    // 仅针对第1条曲线自适应
    // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
    ui->customPlot->graph(1)->rescaleAxes(true);// 在第1条曲线的基础上，自适应第二条曲线
    // Note: we could have also just called customPlot->rescaleAxes(); instead
#endif

    // 重画图像
    ui->customPlot->replot();

    // 允许缩放、拖动、plot可以选着
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    connect(ui->customPlot,SIGNAL(mouseMove(QMouseEvent *)),this,SLOT(myMoveEvent(QMouseEvent *)));
}

void MainWindow::plot_updata_sensor_temp(float *p_value)
{
#if 0
    //  自适应坐标轴
    // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
    ui->customPlot->graph(0)->rescaleAxes();    // 仅针对第1条曲线自适应
    // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
    ui->customPlot->graph(1)->rescaleAxes(true);// 在第1条曲线的基础上，自适应第二条曲线
    // Note: we could have also just called customPlot->rescaleAxes(); instead
#endif

        float x = (float)x_cnt/5;
        x_cnt ++;

        ui->customPlot->graph(0)->addData(x, p_value[0]);
        ui->customPlot->graph(1)->addData(x, p_value[1]);
        ui->customPlot->graph(2)->addData(x, p_value[2]);
        if (!pause_flag)
        {
            if (x > D_MY_PLOT_X_MAX)
            {
                ui->customPlot->xAxis->setRange(0, x+0.1);
            }
            ui->customPlot->replot();
        }
}

void MainWindow::plot_updata_sensor_temp(int x_cnt,float *p_value)
{
        float x = (float)x_cnt/10;

        ui->customPlot->graph(0)->addData(x, p_value[0]);
        ui->customPlot->graph(1)->addData(x, p_value[1]);
        ui->customPlot->graph(2)->addData(x, p_value[2]);
        if (!pause_flag)
        {
            if (x > D_MY_PLOT_X_MAX)
            {
                ui->customPlot->xAxis->setRange(0, x+0.1);
            }
            ui->customPlot->replot();
        }
}

void MainWindow::myMoveEvent(QMouseEvent *event)
{
    if (ui->customPlot->graphCount() == 0)
        return;

    // 获取鼠标坐标，相对父窗体坐标
    int x_pos = event->pos().x();
    int y_pos = event->pos().y();
    // 鼠标坐标转化为CustomPlot内部坐标
    float x_val = ui->customPlot->xAxis->pixelToCoord(x_pos);
    float y_val = ui->customPlot->yAxis->pixelToCoord(y_pos);
//    qDebug("\npos(%d,%d), val(%f,%f)", x_pos, y_pos, x_val, y_val);

#if 0
    // 通过坐标轴范围判断光标是否在点附近
    float x_begin = ui->customPlot->xAxis->range().lower;
    float x_end = ui->customPlot->xAxis->range().upper;
    float y_begin = ui->customPlot->yAxis->range().lower;
    float y_end = ui->customPlot->yAxis->range().upper;
    float x_tolerate = (x_end - x_begin) / 40;
    float y_tolerate = (y_end - y_begin) / 40;

    if (x_val < x_begin || x_val > x_end)
        return;

    // 通过x值查找离曲线最近的一个key值索引
    int index = 0;
    int index_left = ui.customPlot->graph(0)->findBegin(x_val, true);//左边最近的一个key值索引
    int index_right = ui.customPlot->graph(0)->findEnd(x_val, true);//右边
    float dif_left = abs(ui.customPlot->graph(0)->data()->at(index_left)->key - x_val);
    float dif_right = abs(ui.customPlot->graph(0)->data()->at(index_right)->key - x_val);
    if (dif_left < dif_right)
        index = index_left;
    else
        index = index_right;

    x_val = ui.customPlot->graph(0)->data()->at(index)->key;//通过得到的索引获取key值

    int graphIndex=0;//curve index closest to the cursor
    float dx = abs(x_val - ui->customPlot->xAxis->pixelToCoord(x_pos));
    float dy= abs(ui->customPlot->graph(0)->data()->at(index)->value - y_val);

    //通过遍历每条曲线在index处的value值，得到离光标点最近的value及对应曲线索引
    for (int i = 0; i < ui->customPlot->xAxis->graphs().count(); i++)
    {
        if (abs(ui->customPlot->graph(i)->data()->at(index)->value - y_val)<dy)
        {
            dy = abs(ui->customPlot->graph(i)->data()->at(index)->value - y_val);
            graphIndex = i;
        }
    }

    QString strToolTip = "测量点" + QString::number(graphIndex + 1) + ": (";
    strToolTip += QString::number(x_val, 10, 4) + ", ";
    strToolTip += QString::number(ui->customPlot->graph(graphIndex)->data()->at(index)->value) + ")";

    //判断光标点与最近点的距离是否在设定范围内
    if (dy < y_tolerate && dx < x_tolerate)
        QToolTip::showText(cursor().pos(), strToolTip, ui.customPlot);
#endif


#if 0
    //获得x轴坐标位置对应的曲线上y的值
    float line_y_val = ui->customPlot->graph(0)->data()->at(x_val)->value;
    qDebug("\nline_y_val:%f", line_y_val);
    //曲线的上点坐标位置，用来显示QToolTip提示框
    float out_x = ui->customPlot->xAxis->coordToPixel(x_val);
    float out_y = ui->customPlot->yAxis->coordToPixel(line_y_val);
    qDebug("\nout(%f,%f)", out_x, out_y);
#endif

    float out_x = ui->customPlot->pos().x() + x_pos + 2;
    float out_y = ui->customPlot->pos().y() + y_pos + 2;
//    qDebug("\npos(%d,%d)", out_x, out_x);


    QString str,strToolTip;
    str = QString::number(x_val,10,3);
    strToolTip += "x: ";
    strToolTip += str;
    strToolTip += "\n";
    str = QString::number(y_val,10,3);
    strToolTip += "y: ";
    strToolTip += str;
    strToolTip += "\n";
    QToolTip::showText(mapToGlobal(QPoint(out_x,out_y)),strToolTip,ui->customPlot);
}

void MainWindow::on_Button_plot_clear_clicked()
{
    qDebug("%s", __func__);
    x_cnt = 0;

    user_timer.timer_cnt = 0;

    // 仅删除曲线数据
    ui->customPlot->graph(0)->data().data()->clear();
    ui->customPlot->graph(1)->data().data()->clear();
    ui->customPlot->graph(2)->data().data()->clear();
    ui->customPlot->replot();

    ui->customPlot->xAxis->setRange(D_MY_PLOT_X_MIN, D_MY_PLOT_X_MAX);
    ui->customPlot->yAxis->setRange(D_MY_PLOT_Y_MIN, D_MY_PLOT_Y_MAX);

    pause_flag = 0;
    ui->Button_pause->setText("暂停采样");
}

void MainWindow::on_Button_pause_clicked()
{
    if (pause_flag)
    {
        ui->Button_pause->setText("暂停采样");
    }
    else
    {
        ui->Button_pause->setText("恢复采样");
    }
    pause_flag = !pause_flag;
}
