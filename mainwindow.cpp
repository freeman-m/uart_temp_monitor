#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

#include <QTimerEvent>

#include <QSettings>
#include <QFile>

#include <QTextCursor>

#include "qcustomplot.h"

#include "plot.h"

QSerialPort *serial;                    // 定义全局的串口对象
QSerialPort *serial2;
static int open_sta=0;
static int open_sta2=0;

timer_rt user_timer = {0, 0};

// 配置文件保存与加载
void MainWindow::save_windows_parm()
{
    qDebug("%s", __func__);

    settings->setValue("serial1/com", ui->comboBox_port->currentText());
    settings->setValue("serial2/com", ui->comboBox_port_2->currentText());
    settings->setValue("tempparm/temp1", ui->lineEdit_temp1->text().trimmed());
    settings->setValue("tempparm/temp2", ui->lineEdit_temp2->text().trimmed());
    settings->setValue("tempparm/temp3", ui->lineEdit_temp3->text().trimmed());
    settings->setValue("tempparm/temp4", ui->lineEdit_temp4->text().trimmed());

    settings->sync();
}

void MainWindow::read_windows_parm()
{
    qDebug("%s", __func__);
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    qDebug() << QCoreApplication::applicationDirPath();

    ui->comboBox_port->addItem(settings->value("serial1/com").toString());
    ui->comboBox_port_2->addItem(settings->value("serial2/com").toString());

    ui->lineEdit_temp1->setText(settings->value("tempparm/temp1").toString());
    ui->lineEdit_temp2->setText(settings->value("tempparm/temp2").toString());
    ui->lineEdit_temp3->setText(settings->value("tempparm/temp3").toString());
    ui->lineEdit_temp4->setText(settings->value("tempparm/temp4").toString());
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    read_windows_parm();

    ui->lineEdit_temp1->setValidator(new QIntValidator(0,400,this));  // [1,1000]范围内的整数输入
    ui->lineEdit_temp2->setValidator(new QIntValidator(0,400,this));  // [1,1000]范围内的整数输入
    ui->lineEdit_temp3->setValidator(new QIntValidator(0,400,this));  // [1,1000]范围内的整数输入
    ui->lineEdit_temp4->setValidator(new QIntValidator(0,400,this));  // [1,1000]范围内的整数输入

    ui->comboBox_baudrate->setCurrentIndex(2);//2 - 38400
    ui->comboBox_baudrate_2->setCurrentIndex(4);//4 - 115200

    ui->label_7->setStyleSheet("color: red");
    ui->label_9->setStyleSheet("color: green");
    ui->label_10->setStyleSheet("color: blue");

    plot_init();
}


MainWindow::~MainWindow()
{
    save_windows_parm();

    delete ui;
}

// 关闭串口
void MainWindow::serial_close(void)
{
    serial->close();
    ui->Button_open->setText("打开串口");
    open_sta = 0;
}

void MainWindow::on_Button_open_clicked()
{
    // 打开串口
    if (!open_sta)
    {
        serial = new QSerialPort;
        serial->setPortName(ui->comboBox_port->currentText());

        if(serial->open(QIODevice::ReadWrite))              //打开串口成功
        {
            serial->setBaudRate(ui->comboBox_baudrate->currentText().toInt());       //设置波特率

            QObject::connect(serial, &QSerialPort::readyRead, this, &MainWindow::ReadSerialData);

            ui->Button_open->setText("关闭串口");
            open_sta = 1;

            user_timer.timer_idl = startTimer(200);    //单位是 毫秒
        }
        else    //打开失败提示
        {
            QMessageBox::information(this,tr("错误"),tr("打开串口失败！"),QMessageBox::Ok);
        }
    }
    else
    {
        serial_close();
    }
}

// 保存接收区数据为log文件
void MainWindow::on_Button_savelog_clicked()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString fileName = QString("./%1.log").arg(currentDateTime.toString("yyyyMMdd_hhmmss"));
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << ui->DataReceived->toPlainText();
        file.close();
    }
}

// 清除接收区
void MainWindow::on_Button_clearlog_clicked()
{
    ui->DataReceived->clear();
}

// 刷新串口
void MainWindow::on_Button_refresh_clicked()
{
    if (open_sta)
    {
        serial_close();
    }

    ui->comboBox_port->clear();

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();		//获取设备的串口列表

    //冒泡排序法进行排序
    for (int i = 0; i < ports.size();i++)
    {
        for (int j = i + 1; j < ports.size(); j++)
        {
            QString name = ports.at(i).portName();	//i的串口数字
            int portNumI = name.mid(3).toInt();

            name = ports.at(j).portName();			//j的串口数字
            int portNumJ = name.mid(3).toInt();

            if (portNumI > portNumJ)				//ij交换
            {
                ports.swap(i, j);
            }
        }
    }

    foreach (QSerialPortInfo port, ports)
    {
        ui->comboBox_port->addItem(port.portName());
    }
}


void serial_cmd_get_temp(void)
{
    if (!open_sta)
        return;

    char array[8] = {0x01,0x03,0x00,0x00,0x00,0x04,0x44,0x09};  // 巡检仪获取0-3通道温度数据
    serial->write(array, sizeof(array));
}


// 接收数据,使用read () / readLine () / readAll ()
void MainWindow::ReadSerialData()
{
    static QByteArray rec_data;
    static int len;
    float rec_temp[3];
    uchar *uchar_data;

    QByteArray curr_rec = serial->readAll();
    len += curr_rec.length();
    rec_data += curr_rec;
    if(len==13)
    {
//        qDebug() << rec_data.toHex();   //打印16进制

        uchar_data = reinterpret_cast<unsigned char *>(rec_data.data());    // 转换为uchar类型

//        qDebug("%x %x %x %x %x %x", rec_data.at(3), rec_data.at(4), rec_data.at(5), rec_data.at(6), rec_data.at(7), rec_data.at(8));
//        qDebug("%x %x %x %x %x %x", uchar_data[3], uchar_data[4], uchar_data[5], uchar_data[6], uchar_data[7], uchar_data[8]);

        rec_temp[0] = (float)((uchar_data[3]<<8) | uchar_data[4])/10;
        rec_temp[1] = (float)((uchar_data[5]<<8) | uchar_data[6])/10;
        rec_temp[2] = (float)((uchar_data[7]<<8) | uchar_data[8])/10;

        qDebug("[%f,%f,%f]", rec_temp[0], rec_temp[1], rec_temp[2]);

        if (rec_temp[0] > 400)  rec_temp[0] = 399;
        if (rec_temp[1] > 400)  rec_temp[1] = 399;
        if (rec_temp[2] > 400)  rec_temp[2] = 399;

        plot_updata_sensor_temp(rec_temp);

        rec_data.clear();
        len = 0;
    }
}

// 100ms定时器中断
void MainWindow::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == user_timer.timer_idl)
    {
        user_timer.timer_cnt ++;
        serial_cmd_get_temp();
    }

}


// 关闭串口
void MainWindow::serial2_close(void)
{
    serial2->close();
    ui->Button_open_2->setText("打开串口");
    open_sta2 = 0;
}

// 打开串口
void MainWindow::on_Button_open_2_clicked()
{
    // 打开串口
    if (!open_sta2)
    {
        serial2 = new QSerialPort;
        serial2->setPortName(ui->comboBox_port_2->currentText());

        if(serial2->open(QIODevice::ReadWrite))              //打开串口成功
        {
            serial2->setBaudRate(ui->comboBox_baudrate_2->currentText().toInt());       //设置波特率

            QObject::connect(serial2, &QSerialPort::readyRead, this, &MainWindow::ReadSerialData2);

            ui->Button_open_2->setText("关闭串口");
            open_sta2 = 1;

//            user_timer.timer_idl = startTimer(100);    //单位是 毫秒
        }
        else    //打开失败提示
        {
            QMessageBox::information(this,tr("错误"),tr("打开串口失败！"),QMessageBox::Ok);
        }
    }
    else
    {
        serial2_close();
    }
}

// 刷新串口
void MainWindow::on_Button_refresh_2_clicked()
{
    if (open_sta2)
    {
        serial2_close();
    }

    ui->comboBox_port_2->clear();

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();		//获取设备的串口列表

    //冒泡排序法进行排序
    for (int i = 0; i < ports.size();i++)
    {
        for (int j = i + 1; j < ports.size(); j++)
        {
            QString name = ports.at(i).portName();	//i的串口数字
            int portNumI = name.mid(3).toInt();

            name = ports.at(j).portName();			//j的串口数字
            int portNumJ = name.mid(3).toInt();

            if (portNumI > portNumJ)				//ij交换
            {
                ports.swap(i, j);
            }
        }
    }

    foreach (QSerialPortInfo port, ports)
    {
        ui->comboBox_port_2->addItem(port.portName());
    }
}




static const UINT16 sc_protocol_ccitt_crc_table[256] =
{
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


UINT16 protocol_extern_ccitt_crc16_calculation(uchar * p_buff,UINT16 len,UINT16 initial_crc)
{
    while(len>0)
    {
        initial_crc = (initial_crc << 8) ^ sc_protocol_ccitt_crc_table[ ( ( initial_crc>>8 ) ^ *p_buff ) & 0xff ];
        len--;
        p_buff++;
    }

    return initial_crc;
}

#define PROTOCOL_INITIAL_VAL					0x1021

// 协议解析及转义
// 包起始标志位
#define USB_PKT_FLAG							0x7E
// 包转义标志位
#define USB_SHIFT_FLAG							0x5E
// 7e 转义标志
#define USB_SHIFT_FLAG_PKT						0x7D
// 5e 转义标志
#define USB_SHIFT_FLAG_SHIFT					0x5D


// usb协议偏移量
#define USB_MSG_PROTOCOL_LEN_L                  0
#define USB_MSG_PROTOCOL_LEN_H                  1
#define USB_MSG_PROTOCOL_CMD					2
#define USB_MSG_PROTOCOL_ACK					3
#define USB_MSG_PROTOCOL_PDU					4

#define USB_MSG_PKT_HEAD_SIZE                   (USB_MSG_PROTOCOL_PDU)

#define U8TU16( A ,  B  )                       ( ( (UINT16)(A)<<8 ) | (UINT16)(B))
#define U8TU16LTH( A ,  B  )                    ( ( (UINT16)(B)<<8 ) | (UINT16)(A))
void MainWindow::uart_rec_decode(QByteArray data, uint8_t len)
{
    uchar buf[100];
    uint16_t length, crc, crc_cal;
    float device_temp[3];

    qDebug() << data.toHex();

    for (int i=0; i<len; i++)
    {
        buf[i] = (uchar)data[i];
    }

    length = U8TU16LTH(buf[USB_MSG_PROTOCOL_LEN_L], buf[USB_MSG_PROTOCOL_LEN_H]);
    if (length != len)
    {
        qDebug("msg length err %d, %d", length, len);
        return;
    }

    crc = U8TU16LTH( buf[len-2], buf[len-1]);
    crc_cal = protocol_extern_ccitt_crc16_calculation(buf, len-2, PROTOCOL_INITIAL_VAL);
    if (crc_cal != crc)
    {
        qDebug("msg crc err 0x%x, 0x%x", crc, crc_cal);
        return;
    }

    if (buf[USB_MSG_PROTOCOL_CMD] == 0xE1)
    {
        device_temp[0] = (float)U8TU16(buf[6], buf[7])/10;
        device_temp[1] = (float)U8TU16(buf[14], buf[15])/10;
        device_temp[2] = (float)U8TU16(buf[18], buf[19])/10;
    }

    qDebug("device temp [%.1f, %.1f, %.1f]", device_temp[0], device_temp[1], device_temp[2]);
    memset(buf, 0, sizeof(buf));

//    plot_updata_sensor_temp(user_timer.timer_cnt, device_temp);
}


void MainWindow::ReadSerialData2()
{
    static QByteArray decode_data;    // 待解析的16进制数据
    static QByteArray data;
    QByteArray rec_data;

    int start, end, i, index;
    rec_data = serial2->readAll();

    // 显示内容
    if (ui->DataReceived->toPlainText().length() > 100000)
    {
        ui->DataReceived->clear();
    }
    ui->DataReceived->moveCursor(QTextCursor::End);
////    ui->DataReceived->append(tr("接收数据：") + QString(rec_data));

//    ui->DataReceived->insertPlainText(tr("接收数据<--: ") + QString::fromUtf8(rec_data));
//    ui->DataReceived->insertPlainText("\n");
//    ui->DataReceived->moveCursor(QTextCursor::End);
//    ui->DataReceived->insertPlainText(rec_data);

    ui->DataReceived->insertPlainText(rec_data);
    ui->DataReceived->moveCursor(QTextCursor::End);

//    QThread::msleep(10);    // 线程暂停10ms,防止数据读取过快


    data += rec_data;                 // 读取数据

    if(!data.isEmpty())
    {
        if (data.count('\x7E') == 2)
        {
            qDebug() << data.toHex();   //打印16进制
            start = data.indexOf('\x7E');
            end = data.lastIndexOf('\x7E');
            qDebug("%d, %d", start, end);

            i = 0;
            for (index=start+1; index<end; index++)
            {
                if (((uint8_t)data[index] == USB_SHIFT_FLAG) && ((uint8_t)data[index+1] == USB_SHIFT_FLAG_PKT))
                {
                    decode_data[i] = USB_PKT_FLAG;
                    index++;
                }
                else if (((uint8_t)data[index] == USB_SHIFT_FLAG) && ((uint8_t)data[index+1] == USB_SHIFT_FLAG_SHIFT))
                {
                    decode_data[i] = USB_SHIFT_FLAG;
                    index++;
                }
                else
                {
                    decode_data[i] = data[index];
                }
                i++;
            }
            data.clear();
            uart_rec_decode(decode_data, i);
            decode_data.clear();
        }
    }
}


void Delay_MSec(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

void serial2_send_data(QString data)
{
    serial2->write(data.toUtf8(), data.toUtf8().size());
}

void MainWindow::on_Button_set_temp_clicked()
{
    QString sendData;
    sendData.clear();
    if (ui->lineEdit_temp1->text() != NULL)
    {
        sendData += "test heating:1:";
        sendData += ui->lineEdit_temp1->text();
        sendData += "\r\n";
    }
    serial2_send_data(sendData);

    Delay_MSec(50);
    sendData.clear();
    if (ui->lineEdit_temp2->text() != NULL)
    {
        sendData += "test heating:2:";
        sendData += ui->lineEdit_temp2->text();
        sendData += "\r\n";
    }
    serial2_send_data(sendData);

    Delay_MSec(50);
    sendData.clear();
    if (ui->lineEdit_temp3->text() != NULL)
    {
        sendData += "test heating:3:";
        sendData += ui->lineEdit_temp3->text();
        sendData += "\r\n";
    }
    serial2_send_data(sendData);

    Delay_MSec(50);
    sendData.clear();
    if (ui->lineEdit_temp4->text() != NULL)
    {
        sendData += "test heating:4:";
        sendData += ui->lineEdit_temp4->text();
        sendData += "\r\n";
    }
    serial2_send_data(sendData);
}

void MainWindow::on_Button_stop_heat_clicked()
{
    QString sendData = "test heating:stop\r\n";
    serial2_send_data(sendData);
}

void MainWindow::on_Button_cmd_info_clicked()
{
    QString sendData = "info\r\n";
    serial2_send_data(sendData);
}

void MainWindow::on_Button_cmd_res_read_clicked()
{
    QString sendData = "res read\r\n";
    serial2_send_data(sendData);
}

void MainWindow::on_Button_dev_reset_clicked()
{
    QString sendData = "dev reset\r\n";
    serial2_send_data(sendData);
}
