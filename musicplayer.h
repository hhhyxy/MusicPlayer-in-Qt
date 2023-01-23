#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QListWidgetItem>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariant>
#include <QByteArray>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QPixmap>
#include <QSize>
#include <QTableWidgetItem>
#include "music.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MusicPlayer; }
QT_END_NAMESPACE

enum ReplyType {
    ID,
    URL,
    LRC
};
enum PlaylistType {
    ONLINE,
    LOCAL
};

class MusicPlayer : public QWidget
{
    Q_OBJECT

public:
    MusicPlayer(QWidget *parent = nullptr);
    ~MusicPlayer();

protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

private slots:
    void iconActived(QSystemTrayIcon::ActivationReason);

    void on_pushButton_openFile_clicked();  // 打开本地音乐文件夹

    void on_pushButton_lastSong_clicked();  // 下一首

    void on_pushButton_switch_clicked();    // 音乐播放按钮的点击事件

    void on_pushButton_nextSong_clicked();  // 下一首

    void on_pushButton_mode_clicked();  // 播放模式切换按钮的点击事件

    void on_pushButton_volumn_clicked();    // 静音按钮的点击事件

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);    // 本地音乐列表的子项双击事件

    void updateDuration(qint64 duration);// 歌曲信息改变时，更新时长

    void updatePosition(qint64 position);// 更新进度条

    void sliderClicked();   // 进度条的点击事件

    void on_pushButtonClose_clicked();  // 关闭按钮的点击事件

    void on_pushButtonMin_clicked();    // 最小化按钮的点击事件

    void on_pushButtonMax_clicked();    // 最大化按钮的点击事件

    void on_horizontalSlider_Volume_valueChanged(int value);    // 音量条的位置改变事件

    void replyFinished(QNetworkReply *reply); // 搜索歌曲（获取ID）的回复处理事件

    void on_pushButton_search_clicked();    // 搜索按钮的点击事件

    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

    void on_tableWidget_2_itemDoubleClicked(QTableWidgetItem *item);

    void updateCenterLrc(qint64 position);




//    void on_tabWidget_tabBarClicked(int index);

private:
    Ui::MusicPlayer *ui;

    int playlistType;
    QMediaPlaylist *playList; //播放列表
    QMediaPlaylist *playListLocal;  // 本地音乐播放列表
    QMediaPlaylist *playListOnline;  // 在线音乐播放列表
    QMediaPlayer  *mediaPlayer; //播放器
    QStringList musicNameList;  // 音乐名称列表
    QStringList musicNameListLocal;  // 本地音乐名称列表
    QStringList musicNameListOnline;  // 本地音乐名称列表

//    bool isCanPlay; // 是否能播放
    bool isLrc = false;
    int volume; // 音量
    int currentLrcRow = 0;
    QPoint m_mousePoint;    // 鼠标坐标
    QPoint movePoint;   // 窗口移动距离
    bool mousePress;    // 鼠标左键是否按下

    QNetworkAccessManager *network_manager; // 网络请求管理器，用来发送请求和接收应答
    QNetworkRequest *network_request;   // 网络回复

    int id; // 歌曲Id
    QString songName; // 歌曲名称
    QString author; // 歌手
    QString album;  // 专辑
    QString songUrl;    // 歌曲链接
    int songDuration;   // 歌曲时长
    QMap<qint64,QString> lrcMap;   // 歌词
    QList<Music> musicList;    // 搜索结果音乐列表
    QList<Music> songList;  // 播放列表音乐列表
    int replyType;  // 回应类型
    bool isSearchFinished;  // 搜索完成


    void setTrayIcon(); // 设置托盘图标
    void initPlayer();  // 初始化播放器
    void connectSignalsAndSlots();  // 连接信号和槽
    void initTableList();   // 初始化搜索结果列表

    void searchForInfo(QString str);  // 通过搜索获取含歌曲ID的Json
    void searchForUrl();    // 歌曲Url搜索、获取json
    void searchForLrc();    // 歌词搜索、获取json

    void parseJsonForInfo(QByteArray jsonBytes);  // 解析Json获取歌曲信息
    void parseJsonForUrl(QByteArray jsonBytes); // 解析Json获取歌曲Url
    void parseJsonForLrc(QByteArray jsonBytes); // 解析Json获取歌词
    void showLrc();





};
#endif // MUSICPLAYER_H
