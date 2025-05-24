#include "AppManager.h"
#include "images/images.h"

class AppPower : public AppBase
{
private:
    /* data */
public:
    AppPower()
    {
        name = "power";
        title = "电源";
        description = "电源选项";
        image = power_bits;
        _showInList = true;
        _reentrant = false;
    }
    void set();
    void setup();
};
static AppPower app;
extern char latest_appname[];
void AppPower::set(){
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), true);
}
void AppPower::setup()
{
    const menu_item menu[] = {
        {NULL, "取消"},
        {NULL, "重启"},
        {NULL, "关机"},
        {NULL, NULL},
    };
    int res = GUI::menu("电源选项", menu);
    if(res == 1)
    {
        ESP.restart();
    }
    else if(res == 2)
    {
        latest_appname[0] = 0;
        hal.powerOff();
    }
    appManager.goBack();
}
