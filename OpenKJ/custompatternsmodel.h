#ifndef CUSTOMPATTERNSMODEL_H
#define CUSTOMPATTERNSMODEL_H

#include <QAbstractTableModel>

class Pattern
{
private:
    QString name;
    QString artistRegex;
    QString titleRegex;
    QString songIdRegex;
    int artistCaptureGrp;
    int titleCaptureGrp;
    int songIdCaptureGrp;

public:
    QString getArtistRegex() const
    {
        return artistRegex;
    }
    void setArtistRegex(const QString &value) {artistRegex = value;}
    QString getTitleRegex() const
    {
        return titleRegex;
    }
    void setTitleRegex(const QString &value)
    {
        titleRegex = value;
    }
    QString getSongIdRegex() const
    {
        return songIdRegex;
    }
    void setSongIdRegex(const QString &value)
    {
        songIdRegex = value;
    }
    int getArtistCaptureGrp() const
    {
        return artistCaptureGrp;
    }
    void setArtistCaptureGrp(int value)
    {
        artistCaptureGrp = value;
    }
    int getTitleCaptureGrp() const
    {
        return titleCaptureGrp;
    }
    void setTitleCaptureGrp(int value)
    {
        titleCaptureGrp = value;
    }
    int getSongIdCaptureGrp() const
    {
        return songIdCaptureGrp;
    }
    void setSongIdCaptureGrp(int value)
    {
        songIdCaptureGrp = value;
    }

    QString getName() const
    {
        return name;
    }
    void setName(const QString &value)
    {
        name = value;
    }
};

class CustomPatternsModel : public QAbstractTableModel
{
    Q_OBJECT

    QList<Pattern> myData;

public:
    explicit CustomPatternsModel(QObject *parent = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:

    // QAbstractItemModel interface
public:
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Pattern getPattern(int index);
    void loadFromDB();
};

#endif // CUSTOMPATTERNSMODEL_H
