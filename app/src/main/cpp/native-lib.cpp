#include <jni.h>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <random>
#include <mutex>
#include <android/log.h>

#define LOG_TAG "NDK_LIB"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct UIButton {
    float x, y, width, height;
    bool IsClicked(float tx, float ty) const {
        return (tx >= x && tx <= x + width && ty >= y && ty <= y + height);
    }
};

struct FastJoystick {
    float moveX = 0.0f;
    float moveY = 0.0f;
    const float maxRadius = 100.0f;

    void UpdateDirection(float tx, float ty, float cx, float cy) {
        float dx = tx - cx;
        float dy = ty - cy;
        float dist = std::sqrt(dx * dx + dy * dy);
        float div = (dist > maxRadius) ? dist : maxRadius;
        moveX = dx / div;
        moveY = dy / div;
    }

    void Reset() { moveX = moveY = 0.0f; }
};

struct AttackSystem {
    int stack = 0;
    float lastTime = -10000.0f; // very negative so first attack won't be treated as continuation
    std::mt19937 rng;

    AttackSystem()
        : rng(static_cast<unsigned int>(std::random_device{}())) {}

    void RegisterAttack(float now) {
        if (now - lastTime > 1.5f) {
            stack = 0;
        }
        lastTime = now;
        if (stack < 15) stack++;
    }

    float Damage(float base) {
        float dmg = base + static_cast<float>(stack) * 10.0f;
        if (stack >= 10) {
            std::uniform_int_distribution<int> dist(0, 1);
            if (dist(rng) == 1) {
                dmg *= 2.0f;
                LOGI("CRITICAL");
            }
        }
        return dmg;
    }
};

class PlayerController {
    FastJoystick joy;
    AttackSystem atk;
    UIButton btn{1000.0f, 500.0f, 200.0f, 200.0f};
    float time = 0.0f;

public:
    void Touch(float x, float y, bool down) {
        if (!down) {
            joy.Reset();
            return;
        }

        if (x < 640.0f) {
            // Example joystick center at (200,800) — adjust to your layout
            joy.UpdateDirection(x, y, 200.0f, 800.0f);
        } else if (btn.IsClicked(x, y)) {
            Attack();
        }
    }

    void Attack() {
        atk.RegisterAttack(time);
        float d = atk.Damage(100.0f);
        LOGI("Damage %.1f Stack %d", d, atk.stack);
    }

    void Update(float dt) { time += dt; }
};

static PlayerController player;
static std::mutex playerMutex;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_test_NativeLib_tick(JNIEnv*, jobject, jfloat dt) {
    std::lock_guard<std::mutex> lock(playerMutex);
    player.Update(dt);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_test_NativeLib_touch(JNIEnv*, jobject, jfloat x, jfloat y, jboolean down) {
    std::lock_guard<std::mutex> lock(playerMutex);
    player.Touch(x, y, down == JNI_TRUE);
}
