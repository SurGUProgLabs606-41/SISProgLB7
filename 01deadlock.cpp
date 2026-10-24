#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

const int N = 5; // Количество философов

pthread_mutex_t forks[N]; // Вилки
pthread_mutex_t consoleMutex; // Мьютекс для вывода
pthread_mutex_t barrierMutex; // Мьютекс для синхронизации
pthread_cond_t barrierCond; // Условная переменная для синхронизации

int ready = 0;

// Функции для вывода сообщений. Позволят не путать вывод сообщений
void printMessage(int id, const char* message) {
    pthread_mutex_lock(&consoleMutex);
    cout << "Философ " << id + 1 << " " << message << endl;
    pthread_mutex_unlock(&consoleMutex);
}

// Функция для синхронизации философов (ожидания всех философов)
// Позволяет гарантировано воспроизвести deadlock (правая вилка взята другим философом)
void waitForAll() {
    pthread_mutex_lock(&barrierMutex);

    ready++;

    if (ready == N) {
        pthread_cond_broadcast(&barrierCond);
    } else {
        while (ready < N) {
            pthread_cond_wait(&barrierCond, &barrierMutex);
        }
    }

    pthread_mutex_unlock(&barrierMutex);
}

// Функция, реализующая поведение философа
// Выполняется в отдельном потоке для каждого философа
void* philosopher(void* arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % N;

    printMessage(id, "думает");
    sleep(1);

    pthread_mutex_lock(&forks[left]);
    printMessage(id, "взял левую вилку");

    waitForAll();

    printMessage(id, "ждет правую вилку");
    pthread_mutex_lock(&forks[right]);

    printMessage(id, "ест");
    sleep(1);

    pthread_mutex_unlock(&forks[right]);
    pthread_mutex_unlock(&forks[left]);

    printMessage(id, "положил вилки");

    return nullptr;
}

int main() {
    pthread_t threads[N];
    int ids[N];

    pthread_mutex_init(&consoleMutex, nullptr); // Инициализация мьютекса для вывода
    pthread_mutex_init(&barrierMutex, nullptr); // Инициализация мьютекса для синхронизации
    pthread_cond_init(&barrierCond, nullptr); // Инициализация условной переменной для синхронизации

    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&forks[i], nullptr);
    }

    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&threads[i], nullptr, philosopher, &ids[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], nullptr);
    }

    return 0;
}

// g++ 01deadlock.cpp -o a.out -pthread
