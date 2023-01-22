#ifndef MUSIC_H
#define MUSIC_H
#include <QString>

class Music
{
public:
    Music(int id, QString songName, QString author, QString album, int songDuration, QString songUrl = "");

    int getId() const;

    QString getSongName() const;

    QString getAuthor() const;

    QString getAlbum() const;

    QString getSongUrl() const;

    int getSongDuration() const;

    void setSongUrl(const QString &newSongUrl);

private:
    int m_id; // 歌曲Id
    QString m_songName; // 歌曲名称
    QString m_author; // 歌手
    QString m_album;  // 专辑
    QString m_songUrl;    // 歌曲链接
    int m_songDuration;   // 歌曲时长
};

#endif // MUSIC_H
