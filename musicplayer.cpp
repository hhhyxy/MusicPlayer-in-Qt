#include "musicplayer.h"
#include "ui_musicplayer.h"
#include <QFileDialog>
#include <QDebug>
#include <QTime>
#include <QPoint>
#include <QAudioDeviceInfo>
#include <QMediaService>
#include <QAudioOutputSelectorControl>
#include <QAction>
#include <QMenu>


MusicPlayer::MusicPlayer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MusicPlayer)
{
    ui->setupUi(this);
    isCanPlay = false;
    volume = 50;// 初始化音量
    //去掉标题栏 点击任务栏图标显示/隐藏窗口
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint );
    this->setGeometry(253, 32, 1374, 926);
    // 设置托盘图标
    setTrayIcon();
    // 初始化播放器
    initPlayer();
    // 连接信号和槽
    connectSignalsAndSlots();
}

MusicPlayer::~MusicPlayer()
{
    delete ui;
    delete mediaPlayer;
    delete playList;
    mediaPlayer = nullptr;
    playList = nullptr;
}

// 设置托盘图标
void MusicPlayer::setTrayIcon()
{
    //新建一个托盘图标对象 在QSystemTrayIcon()中添加this指针指向musicplayer，以便在关闭窗口时销毁托盘图标
    QSystemTrayIcon *trayicon = new QSystemTrayIcon(this);
    // 设置托盘图标
    trayicon->setIcon(QIcon(QPixmap(":/ico/MusicPlayer.ico")));
    //设置托盘图标提示：鼠标移动到上面会提示文字
    trayicon->setToolTip(QString("MusicPlayer"));
    connect(trayicon, &QSystemTrayIcon::activated, this, &MusicPlayer::iconActived);
    //创建菜单项
    QMenu *traymenu = new QMenu();
    //设置托盘图标右键菜单
    trayicon->setContextMenu(traymenu);

    //创建菜单项内容
//    QAction *action_show = new QAction(tr("显示主界面"),this);
//    QAction *action_quit = new QAction(tr("退出"),this);
    //设置菜单项的图标
//    action_show->setIcon(QIcon("/home/fan/qtproject/taskbardemo/icon.png"));
    //将他们关联起来
//    traymenu->addAction(action_show);
//    traymenu->addAction(action_quit);
    //连接信号与槽：单击菜单项内容执行相应的函数
//    connect(action_show,&QAction::triggered,this,&MusicPlayer::show);
//    connect(action_quit,&QAction::triggered,this,&MusicPlayer::close);

    // 更简便的设置托盘图标菜单方法
    traymenu->addAction("打开主界面",this,&MusicPlayer::show);
    traymenu->addAction("退出",this,&MusicPlayer::close);
    // 显示托盘图标
    trayicon->show();
}

// 初始化播放器
void MusicPlayer::initPlayer()
{
    playList = new QMediaPlaylist(this);//初始化播放列表
    playList->setPlaybackMode(QMediaPlaylist::Loop);//设置播放模式(顺序播放，单曲循环，随机播放等)

    mediaPlayer = new QMediaPlayer(this);//初始化音乐播放器
    mediaPlayer->setPlaylist(playList);  //设置播放列表
    // 获取播放设备列表
    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    // 设置播放设备
    QMediaService *svc = mediaPlayer->service();
    if (svc != nullptr)
    {
        QAudioOutputSelectorControl *out = reinterpret_cast<QAudioOutputSelectorControl*>
                                           (svc->requestControl(QAudioOutputSelectorControl_iid));
        if (out != nullptr)
        {
            out->setActiveOutput(devices[0].deviceName()); // we have to pass deviceID, not the name
            svc->releaseControl(out);
        }
    }

    // 设置音量和音量条
    mediaPlayer->setVolume(volume);
    ui->horizontalSlider_Volume->setValue(volume);
    // 设置进度条不可用
    ui->horizontalSlider->setEnabled(false);
}

// 连接信号和槽
void MusicPlayer::connectSignalsAndSlots()
{
    // 连接信号和槽，更新播放时长和进度条
    connect(mediaPlayer,&QMediaPlayer::positionChanged,this,&MusicPlayer::updatePosition);
    connect(mediaPlayer,&QMediaPlayer::durationChanged,this,&MusicPlayer::updateDuration);
    connect(ui->horizontalSlider,&CustomSlider::customSliderClicked,this,&MusicPlayer::sliderClicked);
}

// 双击托盘图标显示窗口
void MusicPlayer::iconActived(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        //双击托盘显示窗口
        case QSystemTrayIcon::Trigger:
            this->setWindowState(Qt::WindowActive);
            this->show();
            break;
        default:
            break;
    }
}

// 鼠标左键移动：移动窗口，窗口最大化则恢复正常大小
void MusicPlayer::mouseMoveEvent(QMouseEvent *e)
{
    if (this->isMaximized()) {// 窗口最大化则恢复正常大小
        this->showNormal();
        ui->pushButtonMax->setIcon(QIcon(QPixmap(":/ico/max.svg")));
    }

    if(mousePress)// 若是鼠标左键按下则移动窗口
    {
        QPoint movePos = e->globalPos();// 鼠标现在位置
        this->move(movePos - movePoint);// 窗口应当移动到的坐标=鼠标当前全局坐标-鼠标初始相对窗口坐标
    }  
}

// 鼠标左键按下标题栏：记录当前位置
void MusicPlayer::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton && e->y() < (ui->pushButtonClose->height() * 1.5 + ui->pushButtonClose->y()))// 判断是否是鼠标左键按下
    {
        mousePress = true;
    }
    // 鼠标在窗口中的坐标
    movePoint = e->globalPos() - pos();// 鼠标的全局坐标-窗口的坐标
}

// 鼠标左键松开
void MusicPlayer::mouseReleaseEvent(QMouseEvent * e)
{
    Q_UNUSED(e) //没有实质性的作用，只是用来允许e可以不使用，用来避免编译器警告
    mousePress = false;
}

// 鼠标左键双击：缩放窗口，调用on_pushButtonMax_clicked()函数
void MusicPlayer::mouseDoubleClickEvent(QMouseEvent *e)
{
    // 判断是否左键按下，是否按在标题栏
    if(e->button() == Qt::LeftButton && e->y() < (ui->pushButtonClose->height() * 1.5 + ui->pushButtonClose->y())) {
        this->on_pushButtonMax_clicked();
    }
}

// 获取本地音乐文件
void MusicPlayer::on_pushButton_openFile_clicked()
{
    //获取打开的音乐文件夹路径
    QString path = QFileDialog::getExistingDirectory(this, tr("请选择音乐目录"), "D:/Google Chrome Download/Misc");
//    QFileDialog::getOpenFileNames(this, "请选择音乐", "D:/Google Chrome Download/Misc", "Music(*.mp3 *.wav *.jpg)");
    QDir dir(path);

    // 获取音乐文件名
    QStringList musicList = dir.entryList(QStringList()<<"*.mp3"<<"*.wav"<<"*.flac");

    if (musicList.size() > 0) {// 判断是否有音乐可以播放
        isCanPlay = true;
    }else {
        return;
    }
    // 去掉后缀名
    for (int i = 0; i < musicList.size(); i++) {
        QString musicName = musicList.at(i);
        int lastPoint = musicName.lastIndexOf(".");
        ui->listWidget->addItem(musicName.left(lastPoint));
        qDebug()<<musicName.mid(0, musicName.indexOf("-"));
    }
//    ui->listWidget->addItems(musicList);
    // 向播放列表添加歌曲
    for (int i = 0; i < musicList.size(); i++) {
        playList->addMedia(QUrl::fromLocalFile(path + "/" + musicList[i]));//添加歌曲，这里添加的是歌曲的路径
    }
}

// 上一首
void MusicPlayer::on_pushButton_lastSong_clicked()
{
    if (!isCanPlay) {
        return;
    }
    playList->setCurrentIndex(playList->previousIndex());
    mediaPlayer->play();//播放歌曲
    ui->pushButton_switch->setIcon(QIcon(QPixmap(":/ico/Player, pause.svg")));
}

// 音乐开关控制
void MusicPlayer::on_pushButton_switch_clicked()
{
    if (QMediaPlayer::PlayingState == mediaPlayer->state()) {// 音乐暂停
        mediaPlayer->pause();
        ui->pushButton_switch->setIcon(QIcon(QPixmap(":/ico/Player, play.svg")));
        ui->pushButton_switch->setToolTip(tr("播放"));
    }else if (isCanPlay) { // 播放音乐
        mediaPlayer->play();
        ui->pushButton_switch->setIcon(QIcon(QPixmap(":/ico/Player, pause.svg")));
        ui->pushButton_switch->setToolTip(tr("暂停"));
    }
}

// 下一首
void MusicPlayer::on_pushButton_nextSong_clicked()
{
    if (!isCanPlay) {
        return;
    }
    playList->setCurrentIndex(playList->nextIndex());
    mediaPlayer->play();//播放歌曲
    ui->pushButton_switch->setIcon(QIcon(QPixmap(":/ico/Player, pause.svg")));
}

// 改变播放模式
void MusicPlayer::on_pushButton_mode_clicked()
{
    switch (playList->playbackMode()) {
        //列表循环->顺序播放
        case QMediaPlaylist::Loop:
            playList->setPlaybackMode(QMediaPlaylist::Sequential);
            ui->pushButton_mode->setIcon(QIcon(QPixmap(":/ico/order-play-fill.svg")));
            ui->pushButton_mode->setToolTip(tr("顺序播放"));
            break;

        //顺序播放->随机播放
        case QMediaPlaylist::Sequential:
            playList->setPlaybackMode(QMediaPlaylist::Random);
            ui->pushButton_mode->setIcon(QIcon(QPixmap(":/ico/random.svg")));
            ui->pushButton_mode->setToolTip(tr("随机播放"));
            break;

        //随机播放->单曲循环
        case QMediaPlaylist::Random:
            playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
            ui->pushButton_mode->setIcon(QIcon(QPixmap(":/ico/repeat-one-line.svg")));
            ui->pushButton_mode->setToolTip(tr("单曲循环"));
            break;

        //单曲循环->列表循环
        case QMediaPlaylist::CurrentItemInLoop:
            playList->setPlaybackMode(QMediaPlaylist::Loop);
            ui->pushButton_mode->setIcon(QIcon(QPixmap(":/ico/repeat.svg")));
            ui->pushButton_mode->setToolTip(tr("列表循环"));
            break;

        case QMediaPlaylist::CurrentItemOnce:
            break;
    }
}

// 静音开关
void MusicPlayer::on_pushButton_volumn_clicked()
{
    if (0 < mediaPlayer->volume()) {
        mediaPlayer->setVolume(0);
        ui->horizontalSlider_Volume->setEnabled(false);
        ui->pushButton_volumn->setIcon(QIcon(QPixmap(":/ico/noVolume.svg")));
        ui->pushButton_volumn->setToolTip(tr("声音"));
    }
    else {
        mediaPlayer->setVolume(volume);
        ui->horizontalSlider_Volume->setEnabled(true);
        ui->pushButton_volumn->setIcon(QIcon(QPixmap(":/ico/volume.svg")));
        ui->pushButton_volumn->setToolTip(tr("静音"));
    }
}

// 双击列表项播放歌曲
void MusicPlayer::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    // 获取双击项的行
    int itemRow = ui->listWidget->row(item);
    // 设置当前播放歌曲
    playList->setCurrentIndex(itemRow);
    // 正在播放歌曲不需要改变按钮
    if (QMediaPlayer::PlayingState == mediaPlayer->state()) {
        mediaPlayer->play();
        return;
    }

    mediaPlayer->play();
    // 改变暂停按钮
    ui->pushButton_switch->setIcon(QIcon(QPixmap(":/ico/Player, pause.svg")));
    ui->pushButton_switch->setToolTip(tr("暂停"));

}

// 更新播放总时长
void MusicPlayer::updateDuration(qint64 duration)
{
    ui->horizontalSlider->setRange(0, duration);//根据播放时长来设置滑块的范围
    ui->horizontalSlider->setEnabled(duration>0);
    ui->horizontalSlider->setSingleStep(1);
    ui->horizontalSlider->setPageStep(duration/20);//以及每一步的步数

    QTime displayTime(0, (duration/60000) % 60,(duration/1000) % 60);
    ui->label -> setText(displayTime.toString("mm:ss"));
}

// 更新进度条和已播放时长
void MusicPlayer::updatePosition(qint64 position)
{
    if(ui->horizontalSlider->isSliderDown())
            return;//如果手动调整进度条，则不处理
    ui->horizontalSlider->setValue(position);//设置滑块位置
    QTime displayTime(0, (ui->horizontalSlider->value()/60000) % 60,(ui->horizontalSlider->value()/1000) % 60);
    ui->label_3->setText(displayTime.toString("mm:ss"));
}

// 点击进度条更新播放进度
void MusicPlayer::sliderClicked()
{
    mediaPlayer->setPosition(ui->horizontalSlider->value());
}

// 关闭窗口
void MusicPlayer::on_pushButtonClose_clicked()
{
    this->close();
}

// 窗口最小化
void MusicPlayer::on_pushButtonMin_clicked()
{
    this->showMinimized();
}

// 窗口最大化
void MusicPlayer::on_pushButtonMax_clicked()
{

    if (this->isMaximized()) {// 恢复正常大小
        this->showNormal();
        ui->pushButtonMax->setIcon(QIcon(QPixmap(":/ico/max.svg")));
    }
    else {// 最大化
        this->showMaximized();
        ui->pushButtonMax->setIcon(QIcon(QPixmap(":/ico/Max2normal.svg")));
    }

}

// 声音进度条控制音量
void MusicPlayer::on_horizontalSlider_Volume_valueChanged(int value)
{
    volume = value;
    mediaPlayer->setVolume(volume);
}

