#include "AppManager.h"
#include "images/images.h"


class AppDebug : public AppBase
{
private:
    /* data */
public:
    AppDebug()
    {
        name = "debug";
        title = "Debug";
        description = "Debug专用";
        image = debug_bits;
        _showInList = false;
        _reentrant = false;
    }
    void set();
    void setup();
};
static AppDebug app;
void AppDebug::set(){  
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), false);
}
void AppDebug::setup()
{
    alarms.alarm();
    int res = GUI::msgbox_number("test", 4, 0);
    display.setCursor(20, 20);
    display.println(res);
    display.display();
    delay(2000);
    appManager.goBack();
}
