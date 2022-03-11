#ifndef LOCATIONLIST_H
#define LOCATIONLIST_H

#include <QWidget>

#include "includefilelist.h"
#include "weatherinfo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LocationList; }
QT_END_NAMESPACE

class LocationList : public QWidget
{
    Q_OBJECT

public:
    LocationList(QWidget *parent = nullptr);
    ~LocationList();

private:
    Ui::LocationList *ui;

    /* Variables */

    /* QT Variables */
    QString city_name;
    //put each locations information list into list (each location has place name, lat, lon)
    QVector<QStringList> locations_info_list;
    QJsonDocument doc;

    /* API KEY */
    const QString APIKEY = "";

    /* Variables for get Json from API */
    QNetworkAccessManager *net_manager_location;
    QNetworkRequest net_req_location;
    QNetworkReply *net_rep_location;


    /* Private Functions */
    void sendRequest();
    void getRequest();
    void addLocInfo(const QJsonObject&, QStringList& );
    void setInformationToDisplay() const;
    void getLocsInfoList();

private slots:
    void getButtonClicked();
    void moveToWeatherDisplay();

};
#endif // LOCATIONLIST_H
