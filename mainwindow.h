#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// 引入串口通信的两个头文件
#include <QtSerialPort/QSerialPort>         // 提供访问串口的功能
#include <QtSerialPort/QSerialPortInfo>     // 提供系统中存在的串口信息

#include "qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QCPItemText *TextTip;

    // 配置文件对象
    QSettings *settings;

    void serial_close(void);
    void serial2_close(void);
    void plot_init();       // 绘图初始化
    void plot_updata_sensor_temp(float *p_value);
    void plot_updata_sensor_temp(int x_cnt,float *p_value);
    void plot_clear();

    void save_windows_parm();
    void read_windows_parm();

private slots:
    void on_Button_open_clicked();

    void on_Button_savelog_clicked();

    void on_Button_refresh_clicked();

    void on_Button_open_2_clicked();

    void on_Button_refresh_2_clicked();

    // 串口槽函数
    void ReadSerialData();                        // 接收数据
    void ReadSerialData2();
    void uart_rec_decode(QByteArray data, uint8_t len);  // 设备温度解析

    void on_Button_clearlog_clicked();

    // 点击选点显示坐标
    void myMoveEvent(QMouseEvent *event);

    void on_Button_plot_clear_clicked();

    void on_Button_pause_clicked();

    void on_Button_set_temp_clicked();

    void on_Button_stop_heat_clicked();

    void on_Button_cmd_info_clicked();

protected:
    void timerEvent(QTimerEvent *e);

private:
    Ui::MainWindow *ui;

private:
    QStringList baudList;                       // 波特率
};

#endif // MAINWINDOW_H
