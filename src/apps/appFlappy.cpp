#include "AppManager.h"
#include "images/images.h"

class AppFlappy : public AppBase {
public:
    AppFlappy() {
        name = "flappy";
        title = "flappy";
        description = "右键跳跃，穿越障碍";
        image = flappy_bits;
    }

    void setup() override {
        display.clearScreen();

        int bird_y = SCREEN_HEIGHT / 2;
        int velocity = 0;
        int gravity = 1;
        int flap_strength = -5;

        int pipe_x = SCREEN_WIDTH;
        int pipe_gap_y = random(30, SCREEN_HEIGHT - 30);
        const int pipe_gap = 40;
        int score = 0;

        bool running = true;

        while (running) {
            // 控制：右键跳跃
            if (isRightClicked()) {
                velocity = flap_strength;
            }

            // 更新状态
            velocity += gravity;
            bird_y += velocity;
            pipe_x -= 2;

            if (pipe_x < -10) {
                pipe_x = SCREEN_WIDTH;
                pipe_gap_y = random(30, SCREEN_HEIGHT - 30);
                ++score;
            }

            // 碰撞检测
            if (bird_y < 0 || bird_y > SCREEN_HEIGHT ||
                (pipe_x < 20 && pipe_x + 10 > 10 &&
                 (bird_y < pipe_gap_y - pipe_gap / 2 || bird_y > pipe_gap_y + pipe_gap / 2))) {
                GUI::msgbox("游戏结束", (String("得分：") + score).c_str());
                break;
            }

            // 绘制
            display.fillScreen(GxEPD_WHITE);
            display.fillRect(pipe_x, 0, 10, pipe_gap_y - pipe_gap / 2, GxEPD_BLACK);
            display.fillRect(pipe_x, pipe_gap_y + pipe_gap / 2, 10, SCREEN_HEIGHT, GxEPD_BLACK);
            display.fillCircle(10, bird_y, 3, GxEPD_BLACK);
            display.display(true);

            delay(10);
        }

        appManager.goBack();
    }

private:
    bool isRightClicked() {
        static bool last = false;
        bool now = hal.btnr.isPressing();
        bool clicked = !last && now;
        last = now;
        return clicked;
    }
};
static AppFlappy app;
