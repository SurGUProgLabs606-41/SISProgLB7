#include <iostream>
#include <pthread.h>
#include <unistd.h>

// Последний философ берёт вилки в обратном порядке, остальные — левую, затем правую. Это разрывает круг ожидания.

using namespace std;

const int N = 5;       // Количество философов и вилок
const int MEALS = 3;   // Количество приёмов пищи

pthread_mutex_t forks[N];      // Мьютексы для вилок
pthread_mutex_t consoleMutex;  // Мьютекс для вывода сообщений

void printMessage(int id, const char* message) {
    pthread_mutex_lock(&consoleMutex); // Блокируем вывод в консоль
    cout << "Философ " << id + 1 << " " << message << endl;
    pthread_mutex_unlock(&consoleMutex); // Разрешаем вывод другим потокам
}

void* philosopher(void* arg) {
    int id = *(int*)arg;       // Получаем номер философа
    int left = id;             // Номер левой вилки
    int right = (id + 1) % N;  // Номер правой вилки

    for (int i = 0; i < MEALS; i++) {
        printMessage(id, "думает");
        usleep(100000); // Думает 0,1 секунды

        // Последний философ берёт вилки в обратном порядке
        int first = (id == N - 1) ? right : left;
        int second = (id == N - 1) ? left : right;

        pthread_mutex_lock(&forks[first]); // Берём первую вилку
        printMessage(id, "взял первую вилку");

        pthread_mutex_lock(&forks[second]); // Берём вторую вилку
        printMessage(id, "взял вторую вилку и ест");

        usleep(150000); // Ест 0,15 секунды

        pthread_mutex_unlock(&forks[second]); // Освобождаем вторую вилку
        pthread_mutex_unlock(&forks[first]);  // Освобождаем первую вилку

        printMessage(id, "закончил есть");
    }

    return nullptr; // Завершаем поток философа
}

int main() {
    pthread_t threads[N]; // Массив потоков
    int ids[N];           // Номера философов

    pthread_mutex_init(&consoleMutex, nullptr); // Создаём мьютекс консоли

    // Создаём мьютексы для всех вилок
    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&forks[i], nullptr);
    }

    // Создаём потоки философов
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&threads[i], nullptr, philosopher, &ids[i]);
    }

    // Ждём завершения всех философов
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], nullptr);
    }

    cout << "Все философы закончили есть" << endl;

    // Удаляем мьютексы после завершения потоков
    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    pthread_mutex_destroy(&consoleMutex);

    return 0;
}

// Компиляция: g++ 03asymmetry.cpp -o a.out -pthread