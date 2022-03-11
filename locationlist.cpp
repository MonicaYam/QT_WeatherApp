#include "locationlist.h"
#include "ui_locationlist.h"


LocationList::LocationList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LocationList)
{
    ui->setupUi(this);

    net_manager_location = new QNetworkAccessManager(this);

    connect(ui->getCityButton, SIGNAL(clicked()), this,
            SLOT(getButtonClicked()));
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)),
                this, SLOT(moveToWeatherDisplay()));
}

LocationList::~LocationList()
{
    delete ui;
}


/*!
 \fn void LocationList::moveToWeatherDisplay()
 When user chose and press an address on the view, it works to change window to display weather information.
*/
void LocationList::moveToWeatherDisplay(){
    int idx = ui->listWidget->currentRow();

    WeatherInfo *wi = new WeatherInfo(locations_info_list[idx]);
    // Show second window
    wi->show();
    // Hide current window
    hide();

}

/*!
 \fn void LocationList::getButtonClicked()
 When user clicked button, it works to get location list and display it on UI
*/
void LocationList::getButtonClicked(){
    if(locations_info_list.size()>0){
        locations_info_list.clear();
    }

    //reset location list
    sendRequest();
    getLocsInfoList();
    setInformationToDisplay();
}


/*!
 \fn void LocationList::sendRequest()
 Send API Request to LocationIQ with the place name that user input on UI.
*/
void LocationList::sendRequest(){
    //milisecond
    constexpr int TIMEOUT_MS = 30000;

    // Get city name from user input
    city_name = ui->cityName_in->text();
    //Remove whitespace from the beginning and ending
    city_name = city_name.trimmed();

    const QString locUrl = QString("https://us1.locationiq.com/v1/search.php?key=%1&q=%2&format=json").arg(APIKEY,city_name);

    QTimer timer;
    timer.setSingleShot(true);

    net_req_location = QNetworkRequest(QUrl::fromUserInput(locUrl));
    net_rep_location= net_manager_location->get(net_req_location);

    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(net_rep_location, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(TIMEOUT_MS);  // use miliseconds
    loop.exec();

    if(timer.isActive()) {
        timer.stop();
        getRequest();
    }else{
        // timeout
        disconnect(net_rep_location, SIGNAL(finished()), &loop, SLOT(quit()));
        net_rep_location->abort();
    }
}


/*!
 \fn void LocationList::getJson()
 Get address list from net_rep_weather (QNetworkReply).
*/
void LocationList::getRequest(){
    if(net_rep_location->error() != QNetworkReply::NoError){
        qDebug() << "Error : " << net_rep_location->errorString();
        QMessageBox::warning(this,"Error",QString("Request[Error] : %1").arg(net_rep_location->errorString()));
    }else{
        //Convert the data from a json doc
        doc = QJsonDocument::fromJson(net_rep_location->readAll());
        net_rep_location->deleteLater();
    }//else
}


/*!
 \fn void  LocationList::setInformationToDisplay()
 Display address list
*/
void LocationList::getLocsInfoList(){
    QStringList list;
    QJsonArray array = doc.array();

    for(const QJsonValueRef& value: array){
        QJsonObject obj = value.toObject();
        addLocInfo(obj, list);

        // If display name started with city name, add location info to locations info list
        // Otherwise, that display name is not city name. then exclude it.
        if( obj.value("display_name").toString().startsWith(city_name, Qt::CaseInsensitive) == true){
            locations_info_list.push_back(list);
        }

        // Clear list
        list.clear();
    }
}


/*!
 \fn void LocationList::addLocInfo(const QJsonObject& obj, QStringList& list)
 Add location info into the list of locations
*/
void LocationList::addLocInfo(const QJsonObject& obj, QStringList& list){
    list.append(obj.value("display_name").toString());
    list.append(obj.value("lat").toString());
    list.append(obj.value("lon").toString());
}


/*!
 \fn void  LocationList::setInformationToDisplay()
 Display address list
*/
void  LocationList::setInformationToDisplay() const{
    //clear listwidget
    ui->listWidget->clear();
    for(auto value : locations_info_list){
        //add place name as item in QListWidget
        ui->listWidget->addItem( value[0] );
    }
}
