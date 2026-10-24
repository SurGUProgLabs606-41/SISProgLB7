#include <iostream>
#include <pthread.h>
#include <unistd.h>

// Если вторая вилка занята, философ освобождает первую и повторяет попытку. Сразу возвращает резултат.

using namespace std;

const int N = 5;       // Количество философов и вилок
const int MEALS = 3;   // Количество приёмов пищи

pthread_mutex_t forks[N];      // Мьютексы для вилок
pthread_mutex_t consoleMutex;  // Мьютекс для вывода сообщений

void printMessage(int id, const char* message) {
    pthread_mutex_lock(&consoleMutex); // Защищаем вывод
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

        while (true) {
            pthread_mutex_lock(&forks[left]); // Берём левую вилку
            printMessage(id, "взял левую вилку");

            // Пытаемся взять правую вилку, не ожидая её освобождения
            if (pthread_mutex_trylock(&forks[right]) == 0) {
                printMessage(id, "взял правую вилку и ест");
                usleep(150000); // Ест 0,15 секунды

                pthread_mutex_unlock(&forks[right]); // Освобождаем правую вилку
                pthread_mutex_unlock(&forks[left]);  // Освобождаем левую вилку

                printMessage(id, "закончил есть");
                break; // Переходим к следующему приёму пищи
            }

            // Если правая вилка занята, освобождаем левую
            pthread_mutex_unlock(&forks[left]);
            printMessage(id, "правая вилка занята, повторяет попытку");
            usleep(100000); // Ждём перед следующей попыткой
        }
    }

    return nullptr; // Завершаем поток философа
}

int main() {
    pthread_t threads[N]; // Массив потоков
    int ids[N];           // Номера философов

    pthread_mutex_init(&consoleMutex, nullptr); // Создаём мьютекс консоли

    // Инициализируем мьютексы вилок
    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&forks[i], nullptr);
    }

    // Запускаем потоки философов
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&threads[i], nullptr, philosopher, &ids[i]);
    }

    // Ожидаем завершения всех потоков
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], nullptr);
    }

    cout << "Все философы закончили есть" << endl;

    // Уничтожаем мьютексы
    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    pthread_mutex_destroy(&consoleMutex);

    return 0;
}

// Компиляция: g++ 04trylock.cpp -o a.out -pthread