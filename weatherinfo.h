#ifndef WEATHERINFO_H
#define WEATHERINFO_H

#include <QWidget>

#include "includefilelist.h"

namespace Ui {
class WeatherInfo;
}

class WeatherInfo : public QWidget
{
    Q_OBJECT

public:
    explicit WeatherInfo(QWidget *parent = nullptr);
    WeatherInfo(const QList<QString>& locationInfo, QWidget *parent = nullptr);
    ~WeatherInfo();

private:
    Ui::WeatherInfo *ui;
    /* Variables */
    int timeout_ms = 30000;

    /* QT Variables*/
    QEventLoop loop_to_wait_rep;
    QString icon_name;
    QStringList loc_info;
    QJsonObject obj;
    QTimer timer;

    /* Variables for get Json from API */
    QNetworkAccessManager *net_manager_weather;
    QNetworkRequest net_req_weather, net_req_curr_weather_img;
    QNetworkReply *net_rep_weather, *net_rep_curr_weather_img;

    /* API KEY */
    const QString WEATHERAPI_KEY="";

    /* Private Functions */
    void RequestWeatherInfo();
    void getWeatherInfo();
    void setWeatherInfo();
    void setTextToQLabel(QLabel*&, const QString&);
    void setWeatherIcon(QLabel*&, const QString&);
    void setWeatherBG(QLabel*&, const QString&);
    void setAllForcastInfo(QList<QStringList>&);
    void setForcastInfo(QStringList&, int&);
    void setCurrentInfo(const QJsonObject&, const QJsonArray&);
    void setCurrentDetails(const QJsonObject&);

    QList<QStringList> getForcastInfo();
    QString getWindDir(const int&);
    QString createTemperatureInfo(const int&, const int&);
    QString createWeatherBGPath(const QString&&);
    QString createWeatherIconPath(const QString&);
    QString getDayOfWeek(const int&&);
    QStringList createDailyForcast(const QJsonObject&);
    QLabel* searchQLabelByName(const QString&);
};

#endif // WEATHERINFO_H
