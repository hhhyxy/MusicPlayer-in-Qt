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
#include <QtNetwork>


MusicPlayer::MusicPlayer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MusicPlayer)
{
    ui->setupUi(this);
//    isCanPlay = false;
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
    //初始化播放列表
    playListLocal = new QMediaPlaylist(this);
    playListOnline = new QMediaPlaylist(this);
    playList = playListOnline;
    playlistType = PlaylistType::ONLINE;

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
    // 初始化网络请求
    network_request = new QNetworkRequest();
    network_manager = new QNetworkAccessManager(this);

    // 初始化搜索结果列表头
    ui->tableWidget->setColumnCount(4);
    QStringList strList;//设置水平表头
    strList <<tr("歌名")<< tr("歌手") << tr("专辑") << tr("时长");
    ui->tableWidget->setHorizontalHeaderLabels(strList);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 初始化播放列表头
    ui->tableWidget_2->setColumnCount(4);
    ui->tableWidget_2->setHorizontalHeaderLabels(strList);
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);
}

// 连接信号和槽
void MusicPlayer::connectSignalsAndSlots()
{
    // 连接信号和槽，更新播放时长和进度条
    connect(mediaPlayer,&QMediaPlayer::positionChanged,this,&MusicPlayer::updatePosition);
    connect(mediaPlayer,&QMediaPlayer::durationChanged,this,&MusicPlayer::updateDuration);
    connect(ui->horizontalSlider,&CustomSlider::customSliderClicked,this,&MusicPlayer::sliderClicked);
    // 处理请求回复
    connect(network_manager, &QNetworkAccessManager::finished, this, &MusicPlayer::replyFinished);
    connect(ui->lineEdit_search, &QLineEdit::returnPressed, this, &MusicPlayer::on_pushButton_search_clicked);
}

// 搜索歌曲，获取歌曲信息，json格式
void MusicPlayer::searchForInfo(QString str)
{
    replyType = ReplyType::ID;
    // 搜索歌曲，获取ID
    QString searchByWord = QString("https://netease.haohao666.top/cloudsearch?keywords=%1").arg(str);
    network_request->setUrl(QUrl(searchByWord));
    network_manager->get(*network_request);
}

// 回应处理
void MusicPlayer::replyFinished(QNetworkReply *reply)
{
//    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    //无错误返回
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray jsonBytes = reply->readAll();  //获取字节
        reply->deleteLater();   // 删除replay对象
//        QString result(jsonBytes);  //转化为字符串
//        qDebug()<<result;
        switch (replyType) {
        case ReplyType::ID:
            parseJsonForInfo(jsonBytes);//解析api接口返回的json， 获取歌曲ID
            break;
        case ReplyType::URL:
            parseJsonForUrl(jsonBytes);
            break;
        default:
            break;
        }
    }
    else
    {
        reply->deleteLater();
        //处理错误
        qDebug()<<"搜索失败";
    }
}

// 解析json，获取歌曲信息（id, 歌名、歌手、专辑、时长）
void MusicPlayer::parseJsonForInfo(QByteArray jsonBytes)
{
    QJsonParseError json_error; // 错误信息
    // 将json解析未编码未UTF-8的json文档
    QJsonDocument parse_doucment = QJsonDocument::fromJson(jsonBytes, &json_error);
    // 错误处理
    if (json_error.error != QJsonParseError::NoError) {
        qDebug() << json_error.errorString();
        return;
    }
    // 将result下的songs数组提取出来
    QJsonArray songsArray = parse_doucment.object().value("result").toObject().value("songs").toArray();

    musicList.clear();// 清空音乐搜索结果
    // 获取所有歌曲的信息
    for (int i = 0; i < songsArray.size(); i++) {
        // 通过 QJsonArray::at(i)函数获取数组下的第i个元素
        QJsonObject song = songsArray.at(i).toObject();
        id = song.value("id").toInt();  // 获取歌曲ID
        songName = song.value("name").toString();   // 获取歌曲名称
        author = song.value("ar").toArray().at(0).toObject().value("name").toString();// 获取歌手名称
        album = song.value("al").toObject().value("name").toString();  // 获取专辑名称
        songDuration = song.value("dt").toInt();
//        qDebug()<<id<<songName<<author<<album;
        // 存储搜索结果
        Music music(id, songName, author, album, songDuration);
        musicList.push_back(music);
    }

    initTableList();   // 向搜索列表中添加搜索结果
}

// 获取歌曲Url、json格式
void MusicPlayer::searchForUrl()
{
    replyType = ReplyType::URL;
    // 搜索歌曲，获取URL
    QString searchById = QString("https://netease.haohao666.top/song/url?id=%1").arg(id);
    network_request->setUrl(QUrl(searchById));
    network_manager->get(*network_request);
}

// 解析json，获取歌曲Url
void MusicPlayer::parseJsonForUrl(QByteArray jsonBytes)
{
    QJsonParseError json_error; // 错误信息
    // 将json解析未编码未UTF-8的json文档
    QJsonDocument parse_doucment = QJsonDocument::fromJson(jsonBytes, &json_error);
    // 错误处理
    if (json_error.error != QJsonParseError::NoError) {
        qDebug() << json_error.errorString();
        return;
    }
    // 将data下的url提取出来
    QJsonValue urlValue = parse_doucment.object().value("data").toArray().at(0).toObject().value("url");
    songUrl = urlValue.toString();
//    qDebug()<<songUrl;
//    playList->addMedia(QUrl(songUrl));
    // 将双击歌曲添加到播放列表首项，并设为当前播放曲目
    playListOnline->insertMedia(0, QUrl(songUrl));
    playListOnline->setCurrentIndex(0);
    // 音乐播放
    playlistType = PlaylistType::ONLINE;
    playList = playListOnline;
    mediaPlayer->setPlaylist(playListOnline);
    if (QMediaPlayer::PlayingState == mediaPlayer->state()) {
        mediaPlayer->play();
        return;
    }
    mediaPlayer->play();
    // 改变暂停按钮
    ui->pushButton_switch->setIcon(QIcon(QPixmap(":/ico/Player, pause.svg")));
    ui->pushButton_switch->setToolTip(tr("暂停"));
}

// 向搜索列表添加搜索结果
void MusicPlayer::initTableList()
{
    // 清空搜索列表
    int rowCount = ui->tableWidget->rowCount();
    for (int i = 0; i < rowCount; i++) {
        ui->tableWidget->removeRow(0);
    }
    // 向搜索列表添加搜索结果
    for (int i = 0; i < musicList.size(); i++) {
        // 获取搜索列表列表行数
        int currentRow = ui->tableWidget->rowCount();
//        qDebug()<<currentRow;
        // 往后插入一行
        ui->tableWidget->insertRow(currentRow);
        // 新建列表项
        QTableWidgetItem *itemName = new QTableWidgetItem(musicList.at(i).getSongName());// 歌名
        QTableWidgetItem *itemAuthor = new QTableWidgetItem(musicList.at(i).getAuthor());// 歌手
        QTableWidgetItem *itemAlbum = new QTableWidgetItem(musicList.at(i).getAlbum());// 专辑
        int duration =  musicList.at(i).getSongDuration();
        QTime displayTime(0, (duration / 60000) % 60, (duration /1000) % 60);
        QTableWidgetItem *itemDuration = new QTableWidgetItem(displayTime.toString("mm:ss"));// 时长
        // 设置列表项不可编辑
        itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
        itemAuthor->setFlags(itemAuthor->flags() & ~Qt::ItemIsEditable);
        itemAlbum->setFlags(itemAlbum->flags() & ~Qt::ItemIsEditable);
        itemDuration->setFlags(itemDuration->flags() & ~Qt::ItemIsEditable);
        // 向行里插入项
        ui->tableWidget->setItem(currentRow, 0, itemName);
        ui->tableWidget->setItem(currentRow, 1, itemAuthor);
        ui->tableWidget->setItem(currentRow, 2, itemAlbum);
        ui->tableWidget->setItem(currentRow, 3, itemDuration);
    }
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

    // 去掉后缀名,往listwidget里添加音乐名
    for (int i = 0; i < musicList.size(); i++) {
        QString musicName = musicList.at(i);
        int lastPoint = musicName.lastIndexOf(".");
        musicNameListLocal.push_back(musicName.left(lastPoint));
        ui->listWidget->addItem(musicName.left(lastPoint));
//        qDebug()<<musicName.mid(0, musicName.indexOf("-"));
    }
    // 向播放列表添加歌曲
    for (int i = 0; i < musicList.size(); i++) {
        playListLocal->addMedia(QUrl::fromLocalFile(path + "/" + musicList[i]));//添加歌曲，这里添加的是歌曲的路径
    }
    ui->tabWidget->setCurrentIndex(3);
}

// 上一首
void MusicPlayer::on_pushButton_lastSong_clicked()
{
//    switch (playlistType) {
//    case PlaylistType::ONLINE:
//        playList = playListOnline;
//        break;
//    case PlaylistType::LOCAL:
//        playList = playListLocal;
//        break;
//    default:
//        break;
//    }
    // 根据播放列表类型，切换播放列表
    if (PlaylistType::ONLINE == playlistType && playList!=playListOnline) {
        playList = playListOnline;
        mediaPlayer->setPlaylist(playList);
    }
    if (PlaylistType::LOCAL == playlistType && playList!=playListLocal)  {
        playList = playListLocal;
        mediaPlayer->setPlaylist(playList);
    }
    // 判断播放列表是否为空
    if (playList->isEmpty()) {
        return;
    }
//    mediaPlayer->setPlaylist(playList);
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
    }else if (!playList->isEmpty()) { // 播放音乐
        mediaPlayer->play();
        ui->pushButton_switch->setIcon(QIcon(QPixmap(":/ico/Player, pause.svg")));
        ui->pushButton_switch->setToolTip(tr("暂停"));
    }
}

// 下一首
void MusicPlayer::on_pushButton_nextSong_clicked()
{
//    if (!isCanPlay) {
//        return;
//    }
//    switch (playlistType) {
//    case PlaylistType::ONLINE:
//        playList = playListOnline;
//        break;
//    case PlaylistType::LOCAL:
//        playList = playListLocal;
//        break;
//    default:
//        break;
//    }
    // 根据播放列表类型，改变播放列表
    if (PlaylistType::ONLINE == playlistType && playList!=playListOnline) {
        playList = playListOnline;
        mediaPlayer->setPlaylist(playList);
    }
    if (PlaylistType::LOCAL == playlistType && playList!=playListLocal) {
        playList = playListLocal;
        mediaPlayer->setPlaylist(playList);
    }
    // 判断播放列表是否为空
    if (playList->isEmpty()) {
        return;
    }
//    mediaPlayer->setPlaylist(playList);
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
            playListLocal->setPlaybackMode(QMediaPlaylist::Sequential);
            playListOnline->setPlaybackMode(QMediaPlaylist::Sequential);
            ui->pushButton_mode->setIcon(QIcon(QPixmap(":/ico/order-play-fill.svg")));
            ui->pushButton_mode->setToolTip(tr("顺序播放"));
            break;

        //顺序播放->随机播放
        case QMediaPlaylist::Sequential:
            playListLocal->setPlaybackMode(QMediaPlaylist::Random);
            playListOnline->setPlaybackMode(QMediaPlaylist::Random);
            ui->pushButton_mode->setIcon(QIcon(QPixmap(":/ico/random.svg")));
            ui->pushButton_mode->setToolTip(tr("随机播放"));
            break;

        //随机播放->单曲循环
        case QMediaPlaylist::Random:
            playListLocal->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
            playListOnline->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
            ui->pushButton_mode->setIcon(QIcon(QPixmap(":/ico/repeat-one-line.svg")));
            ui->pushButton_mode->setToolTip(tr("单曲循环"));
            break;

        //单曲循环->列表循环
        case QMediaPlaylist::CurrentItemInLoop:
            playListLocal->setPlaybackMode(QMediaPlaylist::Loop);
            playListOnline->setPlaybackMode(QMediaPlaylist::Loop);
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

// 双击本地列表项播放歌曲
void MusicPlayer::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    // 获取双击项的行
    int itemRow = ui->listWidget->row(item);

    // 切换播放列表，切换为本地音乐播放列表
    if (PlaylistType::ONLINE == playlistType && playList != playListLocal) {
        playList = playListLocal;
        mediaPlayer->setPlaylist(playListLocal);
    }
    playlistType = PlaylistType::LOCAL;// 当前播放列表类型
    // 设置当前播放歌曲
    playListLocal->setCurrentIndex(itemRow);

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

// 更新播放总时长和正在播放歌曲信息
void MusicPlayer::updateDuration(qint64 duration)
{
    ui->horizontalSlider->setRange(0, duration);//根据播放时长来设置滑块的范围
    ui->horizontalSlider->setEnabled(duration>0);
    ui->horizontalSlider->setSingleStep(1);
    ui->horizontalSlider->setPageStep(duration/20);//以及每一步的步数

    QTime displayTime(0, (duration/60000) % 60,(duration/1000) % 60);
    ui->label->setText(displayTime.toString("mm:ss"));
    switch (playlistType) {
    case PlaylistType::LOCAL:
        musicNameList = musicNameListLocal;
        break;

    case PlaylistType::ONLINE:
        musicNameList = musicNameListOnline;
        break;
    default:
        break;
    }
    QString currentMusicName = musicNameList.at(playList->currentIndex());
    ui->label_musicName->setText(QString("正在播放：%1").arg(currentMusicName));
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

// 搜索按钮
void MusicPlayer::on_pushButton_search_clicked()
{
    ui->lineEdit_search->clearFocus(); //编辑框失去焦点
    // 搜索歌曲信息
    searchForInfo(ui->lineEdit_search->text());
    // 切换到搜索列表页面
    ui->tabWidget->setCurrentIndex(0);
}

// 双击搜索结果列表，播放被双击歌曲并加入播放列表
void MusicPlayer::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    int currentRow = item->row();
//    qDebug()<<item->row();
    id = musicList.at(currentRow).getId(); // 获取当前双击歌曲的ID
//    qDebug()<<id;
    // 获取在线音乐歌曲的名称列表
    musicNameListOnline.push_front(musicList.at(item->row()).getSongName().append("-").append(musicList.at(item->row()).getAuthor()));
    searchForUrl();   // 获取歌曲URl
    // 在列表最前插入一行
    ui->tableWidget_2->insertRow(0);
    // 新建列表项
    QTableWidgetItem *itemName = new QTableWidgetItem(musicList.at(currentRow).getSongName());// 歌名
    QTableWidgetItem *itemAuthor = new QTableWidgetItem(musicList.at(currentRow).getAuthor());// 歌手
    QTableWidgetItem *itemAlbum = new QTableWidgetItem(musicList.at(currentRow).getAlbum());// 专辑
    int duration =  musicList.at(currentRow).getSongDuration();
    QTime displayTime(0, (duration / 60000) % 60, (duration /1000) % 60);
    QTableWidgetItem *itemDuration = new QTableWidgetItem(displayTime.toString("mm:ss"));// 时长
    // 设置列表项不可编辑
    itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
    itemAuthor->setFlags(itemAuthor->flags() & ~Qt::ItemIsEditable);
    itemAlbum->setFlags(itemAlbum->flags() & ~Qt::ItemIsEditable);
    itemDuration->setFlags(itemDuration->flags() & ~Qt::ItemIsEditable);
    // 向行里插入项
    ui->tableWidget_2->setItem(0, 0, itemName);
    ui->tableWidget_2->setItem(0, 1, itemAuthor);
    ui->tableWidget_2->setItem(0, 2, itemAlbum);
    ui->tableWidget_2->setItem(0, 3, itemDuration);
}

// 双击播放列表项，播放歌曲
void MusicPlayer::on_tableWidget_2_itemDoubleClicked(QTableWidgetItem *item)
{
    int itemRow = item->row();// 双击项的行号

    // 改变播放列表，换为在线音乐播放列表
    if (PlaylistType::LOCAL == playlistType && playList != playListOnline) {
        playList = playListOnline;
        mediaPlayer->setPlaylist(playList);
    }
    playlistType = PlaylistType::ONLINE;// 播放列表类型
    playList->setCurrentIndex(itemRow);// 设置当前播放歌曲
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

