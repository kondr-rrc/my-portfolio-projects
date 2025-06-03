#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>

using namespace sf;
using namespace std;

const double k_B = 1.380649e-23;
const double m = 2.65669e-26; //Кислород
const double Pi = 3.14159268;

//Распределение Максвела
Vector2f maxvel(float T) {
    random_device rd;
    mt19937 gen(rd());

    double a = sqrt(k_B * T / m);

    normal_distribution<float> normal_dist(0.0, a);
    float vx = normal_dist(gen);
    float vy = normal_dist(gen);
    float v = sqrt(vx * vx + vy * vy);


    uniform_real_distribution<float> angle_dist(0.0, 2.0 * Pi);
    float alpha = angle_dist(gen);

    float k = 1.5; //коэффицент преобразования скоростей

    Vector2f z = { v * cos(alpha) / k, v * sin(alpha) / k };

    return z;
}

Color tempToColor(float value) {

    float normalized = abs((value - 10000)) / 210000;
    normalized = clamp(normalized, 0.0f, 1.0f);

    Color color;


    if (normalized < 0.4f) {
        // Синий -> фиолетовый
        float t = normalized / 0.2f;
        color.r = static_cast<unsigned char>(t * 128);
        color.g = 0;
        color.b = 255;
    }
    else if (normalized < 0.6f) {
        // Фиолетовый -> красный
        float t = (normalized - 0.4f) / 0.2f;
        color.r = 128 + static_cast<unsigned char>(t * 127);
        color.g = 0;
        color.b = 255 - static_cast<unsigned char>(t * 255);
    }
    else if (normalized < 0.8f) {
        // Красный -> желтый
        float t = (normalized - 0.6f) / 0.2f;
        color.r = 255;
        color.g = static_cast<unsigned char>(t * 255);
        color.b = 0;
    }
    else {
        // Желтый -> белый
        float t = (normalized - 0.8f) / 0.2f;
        color.r = 255;
        color.g = 255;
        color.b = static_cast<unsigned char>(t * 255);
    }

    return color;
}

class Barrier {
public:
    vector <RectangleShape> list;
    Color color{ 128, 128, 128 };

    void add(float x, float y, float lenght, float hight) {
        RectangleShape a({ hight, lenght });
        a.setFillColor(color);
        a.setPosition({ x,y });

        list.push_back(a);
    }

    void draw(RenderWindow& window) {
        for (int i = 0; i < list.size(); i++) {
            window.draw(list[i]);
        }
    }
};

class Particle {
public:
    Vector2f position;
    Vector2f velocity;

    float radius = 3;
    Color color;
    float mass = 1;

    Particle(float x, float y, float vx, float vy) : position(x, y), velocity(vx, vy) {}

    void move(float dt) { position = position + dt * velocity; }

    void draw(RenderWindow& window) {
        CircleShape c(radius);
        c.setFillColor(color);
        c.setPosition(position);
        window.draw(c);
    }

    void contact_check() {
        if (position.y - radius <= 100) {
            velocity.y = -velocity.y;
            position.y = 100 + radius;
        }

        else if (position.y + radius >= 700) {
            velocity.y = -velocity.y;
            position.y = 700 - radius;
        }

        if (position.x - radius <= 100) {
            velocity.x = -velocity.x;
            position.x = 100 + radius;
        }

        else if (position.x + radius >= 700) {
            velocity.x = -velocity.x;
            position.x = 700 - radius;
        }
    }

    void update_color() {
        color = tempToColor(velocity.x * velocity.x + velocity.y * velocity.y);
    }
};

class Gas {
public:
    int n;
    float t;

    vector<Particle> particles;

    void create(int a, float c) {
        n = a;
        t = c;
        for (int i = 0; i < a; i++) {

            random_device rd;
            mt19937 gen(rd());

            uniform_int_distribution<> dist(100 - 1, 300 - 1); //Параметры барьера
            uniform_int_distribution<> distV(-100, 100);

            float r1 = dist(gen);
            float r2 = dist(gen);

            Vector2f v = maxvel(t);


            Particle p(r1, r2, v.x, v.y);
            p.color = tempToColor(300000);

            particles.push_back(p);
        }
    }

    void resolve_collisions() {
        for (int i = 0; i < particles.size(); i++) {
            for (int j = i + 1; j < particles.size(); j++) {
                Particle& p1 = particles[i];
                Particle& p2 = particles[j];

                Vector2f delta = p1.position - p2.position;
                float distance_squared = delta.x * delta.x + delta.y * delta.y;
                float min_distance = p1.radius + p2.radius;

                // Проверка столкновения
                if (distance_squared < min_distance * min_distance && distance_squared > 0.0001f) {
                    float distance = sqrt(distance_squared);
                    Vector2f n = delta / distance; // Нормаль от p2 к p1

                    // Коррекция позиций (разделение частиц)`
                    float overlap = min_distance - distance;
                    p1.position += n * (overlap / 2.0f);
                    p2.position -= n * (overlap / 2.0f);

                    // Проверка направления движения (чтобы частицы сближались)

                    Vector2f dv = p1.velocity - p2.velocity;
                    float dot_product = (dv.x * n.x + dv.y * n.y);


                    if (dot_product < 0) {
                        // Обмен скоростями вдоль нормали
                        Vector2f impulse = dot_product * n;
                        p1.velocity -= impulse;
                        p2.velocity += impulse;

                        // Обновление цветов после изменения скорости
                        p1.update_color();
                        p2.update_color();
                    }
                }
            }
        }
    }

    void move(float dt) {
        for (int i = 0; i < n; i++) {
            particles[i].move(dt);
        }
    }

    void barrier_chek() {
        for (int i = 0; i < n; i++) {
            particles[i].contact_check();
        }
    }

    void draw(RenderWindow& window) {
        for (int i = 0; i < n; i++) {
            particles[i].draw(window);
        }
    }

    vector<int> histogram() {
        vector<int> list(20, 0);

        // Находим максимальную скорость
        float max_v = 0;
        for (const auto& p : particles) {
            float v = sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
            if (v > max_v) max_v = v;
        }

        if (max_v == 0) return list;

        // Распределяем частицы по бинам
        for (const auto& p : particles) {
            float v = sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
            float norm = v / max_v;

            int bin = static_cast<int>(norm * 19);
            bin = std::min(bin, 19);

            list[bin]++;
        }
        return list;
    }

};

class Histogram {
public:
    vector<RectangleShape> shapes;

    void update(vector<int> list) {
        shapes.clear();

        int max_count = *max_element(list.begin(), list.end());
        if (max_count == 0) max_count = 1;

        for (int j = 0; j < 20; j++) {
            RectangleShape a({ 30, -float(list[j]) * 2 });
            a.setPosition({ float(800 + 30 * j), 700 });
            shapes.push_back(a);
        }
    }

    void draw(RenderWindow& window) {
        for (int i = 0; i < 20; i++) {
            window.draw(shapes[i]);
        }
    }
};


int main()
{
    RenderWindow window(VideoMode({ 1600,800 }), "GAS SIMULATION");
    window.setFramerateLimit(60);

    float t0 = 200; //температура в Кельвинах
    int n = 700; //количество молекул

    Barrier box;

    box.add(100, 100, 5, 600);
    box.add(100, 100, 600, 5);
    box.add(700, 100, 600, 5);
    box.add(100, 700, 5, 600 + 5);

    Histogram hist;

    Gas gas;
    gas.create(n, t0);

    Clock clock;
    while (window.isOpen())
    {
        while (const optional event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
                window.close();
        }

        window.clear();

        box.draw(window);

        hist.update(gas.histogram());
        hist.draw(window);


        float dt = clock.restart().asSeconds() * 0.5;

        gas.move(dt);
        gas.resolve_collisions();
        gas.barrier_chek();
        gas.draw(window);

        window.display();
    }
}