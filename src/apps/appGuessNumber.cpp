#include "AppManager.h"

class AppGuessNumber : public AppBase {
public:
    AppGuessNumber() {
        name = "guessnum";
        title = "猜数字";
        description = "逻辑猜数小游戏";
    }

    void setup() override {
        int target = random(0, 100);
        int tries = 0;
        int guess = -1;

        while (guess != target) {
            guess = GUI::msgbox_number("请输入你猜的数字 (0~99)", 2, 0);
            ++tries;
            if (guess < target) {
                GUI::msgbox("提示", "太小了！");
            } else if (guess > target) {
                GUI::msgbox("提示", "太大了！");
            } else {
                GUI::msgbox("恭喜！", (String("你猜中了，共 ") + tries + " 次").c_str());
            }
        }

        appManager.goBack();
    }
};

static AppGuessNumber app;
