#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

const int N = 5; // Количество философов
const int MEALS = 3; // Количество приемов пищи каждым философом

pthread_mutex_t forks[N]; // Вилки
pthread_mutex_t consoleMutex; // Мьютекс для вывода
sem_t* waiter; // Семафор официанта, ограничивающий количество философов за столом

// Вывод сообщений от философа
void printMessage(int id, const char* message) {
    pthread_mutex_lock(&consoleMutex);
    cout << "Философ " << id + 1 << " " << message << endl;
    pthread_mutex_unlock(&consoleMutex);
}

// Функция, реализующая поведение философа с учетом официанта (ограничение количества философов за столом)
void* philosopher(void* arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % N;

    for (int i = 0; i < MEALS; i++) {
        printMessage(id, "думает");
        usleep(100000);

        sem_wait(waiter); // Разрешение от официанта

        pthread_mutex_lock(&forks[left]);
        printMessage(id, "взял левую вилку");

        pthread_mutex_lock(&forks[right]);
        printMessage(id, "взял правую вилку и ест");

        usleep(150000);

        pthread_mutex_unlock(&forks[right]);
        pthread_mutex_unlock(&forks[left]);

        sem_post(waiter); // Освобождение разрешения у официанта
        printMessage(id, "закончил есть");
    }

    return nullptr;
}

int main() {
    pthread_t threads[N];
    int ids[N];

    sem_unlink("/philosopher_waiter");
    waiter = sem_open("/philosopher_waiter",
                      O_CREAT, 0600, 4);

    if (waiter == SEM_FAILED) {
        cerr << "Ошибка создания семафора" << endl;
        return 1;
    }

    pthread_mutex_init(&consoleMutex, nullptr);

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

    cout << "Все философы закончили есть" << endl;

    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    pthread_mutex_destroy(&consoleMutex);
    sem_close(waiter);
    sem_unlink("/philosopher_waiter");

    return 0;
}

// g++ 02waiter.cpp -o a.out -pthread