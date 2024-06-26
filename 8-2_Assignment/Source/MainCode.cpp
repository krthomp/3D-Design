#include <GLFW/glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;
void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE, MULTIHIT };
enum ONOFF { ON, OFF };

class Brick
{
public:
    float red, green, blue;
    float x, y, width, height;
    BRICKTYPE brick_type;
    ONOFF onoff;
    int hit_count;
    int max_hits;

    Brick(BRICKTYPE bt, float xx, float yy, float ww, float hh, float rr, float gg, float bb, int maxHits = 100)
        : brick_type(bt), x(xx), y(yy), width(ww), height(hh), red(rr), green(gg), blue(bb), onoff(ON), hit_count(0), max_hits(maxHits) {}

    void drawBrick()
    {
        if (onoff == ON)
        {
            glColor3d(red, green, blue);
            glBegin(GL_POLYGON);

            glVertex2d(x + width / 2, y + height / 2);
            glVertex2d(x + width / 2, y - height / 2);
            glVertex2d(x - width / 2, y - height / 2);
            glVertex2d(x - width / 2, y + height / 2);

            glEnd();
        }
    }

    void handleHit()
    {
        if (brick_type == MULTIHIT)
        {
            hit_count++;
            if (hit_count >= max_hits)
            {
                onoff = OFF; // remove brick after max_hits
            }
            else
            {
                // Gradually change color to indicate weakening
                float factor = (float)hit_count / max_hits;
                red = 1.0f * (1 - factor) + 1.0f * factor;
                green = 0.5f * (1 + factor) + 1.0f * factor;
                blue = 0.5f * (1 - factor) + 1.0f * factor;
            }
        }
        else
        {
            onoff = OFF;
        }
    }
};

class Circle
{
public:
    float red, green, blue;
    float radius;
    float x, y;
    float speed;
    float angle; // in radians
    int direction;
    double M_PI = 3.14;

    Circle(double xx, double yy, double rr, float rad, float sp, float ang, float r, float g, float b)
        : x(xx), y(yy), radius(rr), red(r), green(g), blue(b), speed(sp), angle(ang), direction(rand() % 8 + 1) {}

    void CheckCollision(Brick* brk)
    {
        if (brk->onoff == OFF)
            return;

        float halfWidth = brk->width / 2;
        float halfHeight = brk->height / 2;
        if ((x + radius > brk->x - halfWidth && x - radius < brk->x + halfWidth) &&
            (y + radius > brk->y - halfHeight && y - radius < brk->y + halfHeight))
        {
            if (brk->brick_type == REFLECTIVE)
            {
                angle = 2 * M_PI - angle;
                x += cos(angle) * 0.03;
                y += sin(angle) * 0.03;
            }
            else
            {
                brk->handleHit();
            }
        }
    }

    void CheckCollisionWithPaddle(Brick* paddle)
    {
        if (paddle->onoff == OFF)
            return;

        float halfWidth = paddle->width / 2;
        float halfHeight = paddle->height / 2;
        if ((x + radius > paddle->x - halfWidth && x - radius < paddle->x + halfWidth) &&
            (y + radius > paddle->y - halfHeight && y - radius < paddle->y + halfHeight))
        {
            angle = 2 * M_PI - angle; // Reflect angle
            speed *= 0.95; // Apply friction to slow down
        }
    }

    void MoveOneStep()
    {
        if (direction == 1 || direction == 5 || direction == 6)  // up
        {
            if (y > -1 + radius)
            {
                y -= speed;
            }
            else
            {
                direction = rand() % 8 + 1;
            }
        }

        if (direction == 2 || direction == 5 || direction == 7)  // right
        {
            if (x < 1 - radius)
            {
                x += speed;
            }
            else
            {
                direction = rand() % 8 + 1;
            }
        }

        if (direction == 3 || direction == 7 || direction == 8)  // down
        {
            if (y < 1 - radius) {
                y += speed;
            }
            else
            {
                direction = rand() % 8 + 1;
            }
        }

        if (direction == 4 || direction == 6 || direction == 8)  // left
        {
            if (x > -1 + radius) {
                x -= speed;
            }
            else
            {
                direction = rand() % 8 + 1;
            }
        }

        // Reflect the ball at the edges of the window
        if (x - radius < -1 || x + radius > 1)
        {
            angle = M_PI - angle;
        }
        if (y - radius < -1 || y + radius > 1)
        {
            angle = 2 * M_PI - angle;
        }

        x += cos(angle) * speed;
        y += sin(angle) * speed;
    }

    void DrawCircle()
    {
        glColor3f(red, green, blue);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            float degInRad = i * DEG2RAD;
            glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
        }
        glEnd();
    }
};

vector<Circle> world;

class Paddle : public Brick {
public:
    Paddle(float xx, float yy, float ww, float hh, float rr, float gg, float bb)
        : Brick(REFLECTIVE, xx, yy, ww, hh, rr, gg, bb) {}

    void moveLeft() {
        if (x - width / 2 > -1) {
            x -= 0.05;
        }
    }

    void moveRight() {
        if (x + width / 2 < 1) {
            x += 0.05;
        }
    }
};

Paddle paddle(0, -0.9, 0.3, 0.05, 1, 1, 1);

int main(void) {
    srand(time(NULL));

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(480, 480, "Enhanced Brick Game", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    vector<Brick> bricks;
    bricks.emplace_back(REFLECTIVE, -0.75, 0.8, 0.2, 0.1, 1, 1, 0);
    bricks.emplace_back(DESTRUCTABLE, -0.25, 0.8, 0.2, 0.1, 0, 1, 0);
    bricks.emplace_back(MULTIHIT, 0.25, 0.8, 0.2, 0.1, 0, 0, 1);
    bricks.emplace_back(REFLECTIVE, 0.75, 0.8, 0.2, 0.1, 1, 0, 0);
    bricks.emplace_back(MULTIHIT, -0.5, 0.6, 0.2, 0.1, 1, 0.5, 0.5);
    bricks.emplace_back(DESTRUCTABLE, 0.0, 0.6, 0.2, 0.1, 0, 1, 1);
    bricks.emplace_back(REFLECTIVE, 0.5, 0.6, 0.2, 0.1, 0.5, 0, 1);
    bricks.emplace_back(DESTRUCTABLE, -0.75, 0.4, 0.2, 0.1, 1, 1, 0);
    bricks.emplace_back(REFLECTIVE, -0.25, 0.4, 0.2, 0.1, 0, 1, 0);
    bricks.emplace_back(MULTIHIT, 0.25, 0.4, 0.2, 0.1, 0, 0, 1);
    bricks.emplace_back(DESTRUCTABLE, 0.75, 0.4, 0.2, 0.1, 1, 0, 0);

    while (!glfwWindowShouldClose(window)) {
        // Setup View
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        processInput(window);

        // Movement and Collision Detection
        for (Circle& circle : world)
        {
            for (Brick& brick : bricks)
            {
                circle.CheckCollision(&brick);
            }
            circle.CheckCollisionWithPaddle(&paddle);
            circle.MoveOneStep();
            circle.DrawCircle();
        }

        // Draw Bricks and Paddle
        for (Brick& brick : bricks)
        {
            brick.drawBrick();
        }
        paddle.drawBrick();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        paddle.moveLeft();
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        paddle.moveRight();
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        double r = rand() / (double)RAND_MAX;
        double g = rand() / (double)RAND_MAX;
        double b = rand() / (double)RAND_MAX;
        double angle = (rand() % 360) * DEG2RAD;
        Circle newCircle(0, 0, 0.02, 0.05, 0.01, angle, r, g, b);
        world.push_back(newCircle);
    }
}
