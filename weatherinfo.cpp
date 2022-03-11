#include "weatherinfo.h"
#include "ui_weatherinfo.h"

//https://unsplash.com/photos/bWtd1ZyEy6w
WeatherInfo::WeatherInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WeatherInfo)
{
    ui->setupUi(this);

}
WeatherInfo::WeatherInfo(const QList<QString>& location_info, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WeatherInfo)
{
    ui->setupUi(this);

    loc_info = location_info;

    // A single-shot timer fires only once
    timer.setSingleShot(true);

    // QNetworkAccessManager allows the application to send network requests and receive replies
    net_manager_weather = new QNetworkAccessManager(this);

    RequestWeatherInfo();

    setWeatherInfo();
    QList<QList<QString>>  forcast_list = getForcastInfo();
    setAllForcastInfo(forcast_list);
    qDebug() << forcast_list[0];

    // Delete QNetworkAccessManager
    delete net_manager_weather;
}


WeatherInfo::~WeatherInfo()
{
    delete ui;
}


/*!
 \fn void WeatherInfo::RequestWeatherInfo()
 Send API Request to openweathermap
*/
void WeatherInfo::RequestWeatherInfo(){
    const QString locUrl = QString("https://api.openweathermap.org/data/2.5/onecall?lat=%1&lon=%2&exclude=hourly,minutely&units=metric&appid=%3")
                                .arg(loc_info[1], loc_info[2], WEATHERAPI_KEY);
    timer.setSingleShot(true);

    qDebug() << locUrl;
    // Set Request with URL(locUrl)
    net_req_weather = QNetworkRequest(QUrl::fromUserInput(locUrl));
    // Post request and get reply
    net_rep_weather= net_manager_weather->get(net_req_weather);

    // Wait until QNetworkReply's status is finished
    connect(&timer, SIGNAL(timeout()), &loop_to_wait_rep, SLOT(quit()));
    connect(net_rep_weather, &QNetworkReply::finished, &loop_to_wait_rep, &QEventLoop::quit);
    timer.start(timeout_ms);
    loop_to_wait_rep.exec();

    if(timer.isActive()) {
        //timer stop here
        timer.stop();
        getWeatherInfo();
    }else{
        // timeout
        disconnect(net_rep_weather, SIGNAL(finished()), &loop_to_wait_rep, SLOT(quit()));
        net_rep_weather->abort();
    }
}

/*!
 \fn void WeatherInfo::getWeatherInfo()
 Get weather information from net_rep_weather (QNetworkReply).
 QNetworkReplyclass contains the data and headers for a request sent with QNetworkAccessManager.
*/
void WeatherInfo::getWeatherInfo(){
    if(net_rep_weather->error() == QNetworkReply::NoError){
        // Convert the data from a json to a QObject
        QJsonDocument doc = QJsonDocument::fromJson(net_rep_weather->readAll());
        obj = doc.object();

        // Delete QNetworkReply later
        net_rep_weather->deleteLater();
    }else{
        // QNetworkReply returns Error
        qDebug() << "Error : " << net_rep_weather->errorString();
        QMessageBox::warning(this,"Error",QString("Request[Error] : %1").arg(net_rep_weather->errorString()));
    }
}


/*!
 \fn void WeatherInfo::setWeatherInfo()
 Set current weather information and details to QLabel
*/
void WeatherInfo::setWeatherInfo(){
    QJsonObject current = obj.value("current").toObject();
    QJsonArray weather = current.value("weather").toArray();
    icon_name = weather[0].toObject().value("icon").toString();

    setCurrentInfo(current, weather);
    setCurrentDetails(current);
}


/*!
 \fn void WeatherInfo::setCurrentInfo(const QJsonObject& current, const QJsonArray& weather)
 Set current weather information on ui
*/
void WeatherInfo::setCurrentInfo(const QJsonObject& current, const QJsonArray& weather){
    // Set place name information
    ui->placeName_lbl->setText(loc_info[0]);

    //set background
    QString img_path = createWeatherBGPath(weather[0].toObject().value("description").toString());
    setWeatherBG(ui->backGroundImg, img_path);

    // Set information
    QString description = weather[0].toObject().value("description").toString();
    setTextToQLabel(ui->c_weather, description);

    int temp = qRound(current.value("temp").toDouble());
    QString temp_str = QString::number(temp)+" ℃";
    setTextToQLabel(ui->c_temp, temp_str);
}


/*!
 \fn void WeatherInfo::setCurrentDetails(const QJsonObject& current)
 Set current weather details on ui
*/
void WeatherInfo::setCurrentDetails(const QJsonObject& current){
    // Set humidity
    QString humidity = QString::number(current.value("humidity").toDouble());
    setTextToQLabel(ui->humidity, humidity);

    // Set uvi
    QString uvi = QString::number(current.value("uvi").toDouble());
    setTextToQLabel(ui->uvi, uvi);

    // Set feels like temperature
    int feels_temp = qRound(current.value("feels_like").toDouble());
    QString feels_temp_str = QString::number(feels_temp)+" ℃";
    setTextToQLabel(ui->feelTemp, feels_temp_str);

    // Set wind speed
    QString windspd = QString::number(current.value("wind_speed").toDouble());
    setTextToQLabel(ui->windSpd, windspd);

    // Get wind degree
    int windDeg = current.value("wind_deg").toInt();
    // Get wind direction from wind degree
    QString windDir = getWindDir(windDeg);
    // Set wind direction
    setTextToQLabel(ui->windDir, windDir);
}


/*!
 \fn void WeatherInfo::setTextToQLabel(QLabel*& label, const QString& content)
 Set text to QLabel
*/
void WeatherInfo::setTextToQLabel(QLabel*& label, const QString& content){
    label->setText(content);
}


/*!
 \fn void WeatherInfo::setWeatherBG(QLabel*& bg_label, const QString& bg_path)
 Set weather back ground image on ui (bg_label)
*/
void WeatherInfo::setWeatherBG(QLabel*& bg_label, const QString& bg_path){
    QPixmap pixmap = QPixmap(bg_path);
    bg_label->setFixedSize(623, 445);
    bg_label->setPixmap(pixmap);
}


/*!
  \fn QString WeatherInfo::createWeatherBGPath(const QString&& name)
 Create weather image path for background from weather direction
*/
QString WeatherInfo::createWeatherBGPath(const QString&& name){
    QString img_path=":/images/";

    if ( name.startsWith("clear", Qt::CaseInsensitive) == true ) {
        img_path += "sunny.png";
    }else if ( name.endsWith("clouds", Qt::CaseInsensitive) == true ) {
        img_path += "cloud.png";
    }else if ( name.endsWith("rain", Qt::CaseInsensitive) == true ) {
        img_path +="rain.png";
    }else if ( name.contains("thunderstorm", Qt::CaseInsensitive) == true ){
        img_path += "thunderStorm.png";
    }else{
        img_path +="sunny.png";
    }

    return img_path;
}


/*!
  \fn QString WeatherInfo::getWindDir(const int& windDeg)
  Get wind direction as QString that using \a windDeg (int) from weather information.
*/
QString WeatherInfo::getWindDir(const int& windDeg){
    //create wind direction list
    const QStringList dirArray={"N","NNE","NE","ENE","E","ESE", "SE", "SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"};

    int dirIdx = qRound(double(windDeg)/22.5) % 16;
    return dirArray[dirIdx];
}


/*!
  \fn QList<QStringList> WeatherInfo::getForcastInfo()
  Get 7 days forcast information list
*/
QList<QStringList>  WeatherInfo::getForcastInfo(){
    QList<QStringList> forcast_list;
    QStringList daily_forcast;
    QJsonArray daily_array = obj.value("daily").toArray();  //array

    for(QJsonValueRef daily_info_ref : daily_array){
        QJsonObject daily_info = daily_info_ref.toObject();

        // Create and get daily info list
        daily_forcast = createDailyForcast(daily_info);

        // Add daily forcast info liston forcast list
        forcast_list.append(daily_forcast);

        // Clear daily forcast
        daily_forcast.clear();
    }

    return forcast_list;
}


/*!
  \fn QStringList WeatherInfo::createDailyForcast(const QJsonObject& daily_info)
  Get one day forcast
*/
QStringList WeatherInfo::createDailyForcast(const QJsonObject& daily_info){
    QStringList daily_forcast_info;
    QJsonObject temp = daily_info.value("temp").toObject();
    QJsonArray weather = daily_info.value("weather").toArray();

    // Date
    daily_forcast_info.append(QString::number(daily_info.value("dt").toInt()));
    // Minimum temperature
    daily_forcast_info.append(QString::number(temp.value("min").toDouble()));
    // Max temperature
    daily_forcast_info.append(QString::number(temp.value("max").toDouble()));
    // Weather description
    daily_forcast_info.append(weather[0].toObject().value("description").toString());

    return daily_forcast_info;
}


/*!
  \fn void WeatherInfo::setForcastInfo(QList<QStringList>& forcast_list)
  Set all forcast info on UI
*/
void  WeatherInfo::setAllForcastInfo(QList<QStringList>& forcast_list){
    for(int i=0; i < forcast_list.size(); ++i){
        setForcastInfo(forcast_list[i], i);
    }
}


/*!
  \fn void WeatherInfo::setForcastInfo(QStringList& forcast, int& num)
  Set daily forcast on UI
*/
void WeatherInfo::setForcastInfo(QStringList& forcast, int& num){

    // Create name for day, temperature, and weather label on ui
    const QString day_name = "dayname_" + QString::number(num);
    const QString temp_name = "temp_" + QString::number(num);
    const QString weather_name = "weather_" + QString::number(num);

    // Get QLabel for each information
    QLabel *weather_label = searchQLabelByName(weather_name);
    QLabel *day_label = searchQLabelByName(day_name);
    QLabel *temp_label = searchQLabelByName(temp_name);

    // Set day of week
    QString dayOfWeek = getDayOfWeek(forcast[0].toInt());
    setTextToQLabel(day_label, dayOfWeek);

    // Set temperature info
    QString temp_info = createTemperatureInfo(qRound(forcast[1].toDouble()), qRound(forcast[2].toDouble()));
    setTextToQLabel(temp_label, temp_info);

    // Set weather icon
    QString weather_icon_path = createWeatherIconPath(forcast[3]);
    setWeatherIcon(weather_label, weather_icon_path);
}


/*!
  \fn QString WeatherInfo::getDayOfWeek(const int&& unix_time)
  Get the name of day of week.
*/
QString WeatherInfo::getDayOfWeek(const int&& unix_time){
    QDateTime time =  time.fromSecsSinceEpoch(unix_time);
    QString date = time.toString();

    return date.split(" ").first();
}


/*!
  \fn void WeatherInfo::setWeatherIcon(QLabel*& weather_label, const QString& path)
  Set weather icon on UI
*/
void WeatherInfo::setWeatherIcon(QLabel*& weather_label, const QString& path){
    QPixmap pixmap = QPixmap(path);
    weather_label->setFixedSize(32, 32);
    weather_label->setPixmap(pixmap);
    weather_label->setAlignment(Qt::AlignCenter);
}


/*!
  \fn QLabel*  WeatherInfo::searchQLabelByName(const QString& label_name)
  Search QLabel by its name and return it
*/
QLabel*  WeatherInfo::searchQLabelByName(const QString& label_name){
    return WeatherInfo::findChild<QLabel*>(label_name);
}


/*!
  \fn QString WeatherInfo::createTemperatureInfo(const int& min, const int& max)
  SCreate temperature info as QString. Return a value like 1℃ | 12℃ as QString
*/
QString WeatherInfo::createTemperatureInfo(const int& min, const int& max){
    return QString::number(min) + "℃ | " + QString::number(max) + "℃";
}


/*!
  \fn QString  WeatherInfo::createWeatherIconPath(const QString& name)
  Create weather icon path
  Check weather name and add weather icon's name on icon_path
  Then, return that path variable
*/
QString  WeatherInfo::createWeatherIconPath(const QString& name){
    QString icon_path=":/images/";

    if ( name.startsWith("clear", Qt::CaseInsensitive) == true ) {
        icon_path += "sunny_icon.png";
    }else if ( name.endsWith("clouds", Qt::CaseInsensitive) == true ) {
        icon_path += "cloud_icon.png";
    }else if ( name.endsWith("rain", Qt::CaseInsensitive) == true ) {
        icon_path += "rain_icon.png";
    }else if ( name.contains("thunderstorm", Qt::CaseInsensitive) == true ){
        icon_path += "thunder_icon.png";
    }else{
        icon_path += "snow_icon.png";
    }

    return icon_path;
}


