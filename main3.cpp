// Założenia do III etapu:
// Korekta poprzedniego programu, oczekujący w kolejce przed dystrybutorem może wybrać inne nie zajęte stanowisko końcowe.

#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <ncurses.h>
#include <condition_variable>
#include <mutex>

using namespace std;

mutex mtx5;
mutex mtx6;
mutex mtx7;
mutex passengersMutex;
mutex stanowiskaMutex;
mutex pathsMutex;

std::mutex globalQueCountMtx;

condition_variable cv;
condition_variable cv5;
condition_variable cv6;
condition_variable cv7;
condition_variable cvPaths;

bool isDistributionWork = false;

int globalQueue;

// Stan zajętości stanowisk
bool stanowisko1_zajete = false;
bool stanowisko2_zajete = false;
bool stanowisko3_zajete = false;

// Stan zajętości ścieżek
bool path1_zajete = false;
bool path2_zajete = false;
bool path3_zajete = false;

int currentPath = 1;

struct Passenger {
    string name;
    int x_pos;
    int y_pos;
    double travel_speed;
    bool QueueStatus;
    int stanowisko; 
    int path;
};

char generateRandCH() {
    const char charset[] = "!@#$%^&*?ABCDEFGHIJKLMOPRSTUWXYZQ0123456789abcdefghijklmnoprstuwxyzq";
    const int max_index = sizeof(charset) - 1;
    return charset[rand() % max_index];
}

struct PassengerManager {
    vector<Passenger> passengers;
    int path_number;
    mutex passengersMutex;

    void addPassenger(const Passenger& passenger) {
        passengers.push_back(passenger);
    }

    void removePassenger(const string& name) {
        passengers.erase(remove_if(passengers.begin(), passengers.end(), [&](const Passenger& p) { return p.name == name; }), passengers.end());
    }

    void displayPassengers() {
        clear();
        int path_nr = path_number;

        int color_code = 3;

        for (const auto& passenger : passengers) {
            attron(COLOR_PAIR(color_code));
            mvprintw(passenger.y_pos, passenger.x_pos, "%s", passenger.name.c_str());
            attroff(COLOR_PAIR(color_code));

            color_code = (color_code % 6) + 1;
        }

        mvprintw(3, 1, "KLIENCI");

        mvprintw(9, 17, "================================================");
        mvprintw(12, 17, "================================================");

        if (path_nr == 1) {
            mvprintw(10, 19, "$"); mvprintw(10, 21, "$");
            mvprintw(10, 39, "x"); mvprintw(10, 41, "x");
            mvprintw(10, 59, "x"); mvprintw(10, 61, "x");
        }
        else if (path_nr == 2) {
            mvprintw(10, 19, "x"); mvprintw(10, 21, "x");
            mvprintw(10, 39, "$"); mvprintw(10, 41, "$");
            mvprintw(10, 59, "x"); mvprintw(10, 61, "x");
        }
        else if (path_nr == 3) {
            mvprintw(10, 19, "x"); mvprintw(10, 21, "x");
            mvprintw(10, 39, "x"); mvprintw(10, 41, "x");
            mvprintw(10, 59, "$"); mvprintw(10, 61, "$");
        }

        mvprintw(10, 1, "DYSTRYBUTOR");

        mvprintw(16, 19, "(---)");
        mvprintw(16, 39, "(---)");
        mvprintw(16, 59, "(---)");

        mvprintw(16, 1, "STANOWISKA");

        refresh();
    }
};

void wybierzStanowisko(Passenger& passenger) {
    unique_lock<mutex> lock(stanowiskaMutex);
    while (true) {
        if (!stanowisko1_zajete) {
            stanowisko1_zajete = true;
            passenger.stanowisko = 1;
            return;
        } else if (!stanowisko2_zajete) {
            stanowisko2_zajete = true;
            passenger.stanowisko = 2;
            return;
        } else if (!stanowisko3_zajete) {
            stanowisko3_zajete = true;
            passenger.stanowisko = 3;
            return;
        }
        cv.wait(lock); // Czekaj na zwolnienie stanowiska
    }
}

void zwolnijStanowisko(Passenger& passenger) {
    unique_lock<mutex> lock(stanowiskaMutex);
    if (passenger.stanowisko == 1) {
        stanowisko1_zajete = false;
    } else if (passenger.stanowisko == 2) {
        stanowisko2_zajete = false;
    } else if (passenger.stanowisko == 3) {
        stanowisko3_zajete = false;
    }
    cv.notify_all(); // Powiadomienie innych wątków o zwolnieniu stanowiska
}

void wybierzPath(Passenger& passenger) {
    unique_lock<mutex> lock(pathsMutex);
    while (true) {
        if (currentPath == 1 && !path1_zajete) {
            path1_zajete = true;
            passenger.path = 1;
            return;
        } else if (currentPath == 2 && !path2_zajete) {
            path2_zajete = true;
            passenger.path = 2;
            return;
        } else if (currentPath == 3 && !path3_zajete) {
            path3_zajete = true;
            passenger.path = 3;
            return;
        }
        cvPaths.wait(lock); // Czekaj na zwolnienie ścieżki lub zmianę dystrybutora
    }
}

void zwolnijPath(Passenger& passenger) {
    unique_lock<mutex> lock(pathsMutex);
    if (passenger.path == 1) {
        path1_zajete = false;
    } else if (passenger.path == 2) {
        path2_zajete = false;
    } else if (passenger.path == 3) {
        path3_zajete = false;
    }
    cvPaths.notify_all(); // Powiadomienie innych wątków o zwolnieniu ścieżki
}

void movePassenger(const string& name, double travel_speed, PassengerManager& passengerManager, int screen_width) {
    Passenger* passengerPtr = nullptr;

    int color_code = rand() % 6 + 1;

    while (true) {
        auto it = find_if(passengerManager.passengers.begin(), passengerManager.passengers.end(), [&](const Passenger& p) { return p.name == name; });
        if (it != passengerManager.passengers.end()) {
            passengerPtr = &(*it);
            passengerPtr->y_pos++;

            attron(COLOR_PAIR(color_code));
            mvprintw(passengerPtr->y_pos, passengerPtr->x_pos, "%s", passengerPtr->name.c_str());
            attroff(COLOR_PAIR(color_code));

            if (passengerPtr->y_pos == 5) {
                unique_lock<mutex> lock(mtx5);
                if (isDistributionWork) {
                    passengerPtr->QueueStatus = true;
                    cv5.wait(lock, [] { return !isDistributionWork; });
                }
                isDistributionWork = false;
            }

            else if (passengerPtr->y_pos == 6) {
                unique_lock<mutex> lock(mtx6);
                if (isDistributionWork) {
                    passengerPtr->QueueStatus = true;
                    cv6.wait(lock, [] { return !isDistributionWork; });
                }
                isDistributionWork = false;
                cv5.notify_all();
            }

            else if (passengerPtr->y_pos == 7) {
                unique_lock<mutex> lock(mtx7);
                if (isDistributionWork) {
                    passengerPtr->QueueStatus = false;
                    cv7.wait(lock, [] { return !isDistributionWork; });
                }
                isDistributionWork = true;
                cv5.notify_all();
                cv6.notify_all();
            }

            if (passengerPtr->y_pos == 10) {
                wybierzPath(*passengerPtr); // Wybór ścieżki dla pasażera
                if (passengerPtr->path == 1) {
                    passengerPtr->x_pos -= 20;
                }
                else if (passengerPtr->path == 3) {
                    passengerPtr->x_pos += 20;
                }
                passengerPtr->y_pos++;
            }

            if (passengerPtr->y_pos == 12) {
                wybierzStanowisko(*passengerPtr); // Wybór stanowiska dla pasażera
            }

            if(passengerPtr->y_pos == 13){
                passengerPtr->QueueStatus = true;
                isDistributionWork = false;
                cv5.notify_all();
                cv6.notify_all();
                cv7.notify_all();
            }

            if (passengerPtr->y_pos == 15) {
                this_thread::sleep_for(chrono::seconds(1));
                zwolnijStanowisko(*passengerPtr); // Zwolnienie stanowiska po obsłudze
            }

            if (passengerPtr->y_pos == 16) {
                zwolnijPath(*passengerPtr); // Zwolnienie ścieżki po obsłudze
                passengerManager.removePassenger(name);
                return;
            }
        }
        else {
            return;
        }
        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(1000 / travel_speed)));
    }
}

void QueueMonitoring(PassengerManager& passengerManager, bool& stop_flag){
    while(!stop_flag || !passengerManager.passengers.empty()){
        int count = 0;
        {
            lock_guard<mutex> lock(passengerManager.passengersMutex);
            for (const auto& passenger : passengerManager.passengers){
                if (passengerManager.path_number == 2)
                    if (passenger.QueueStatus) count++;
            }
        }

        globalQueCountMtx.lock();
        globalQueue = count;
        globalQueCountMtx.unlock();
    }
}

// Odpowiada za dynamiczne zarządzanie dostępnością ścieżek dla pasażerów, zapewniając, że ścieżki są zmieniane co określony czas (500 ms) i unikając jednoczesnego korzystania z zajętych ścieżek
void selectPath(PassengerManager& passengerManager, bool& stop_flag) {
    while (!stop_flag || !passengerManager.passengers.empty()) { 
        unique_lock<mutex> lock(pathsMutex);
        currentPath = (currentPath % 3) + 1;
        
        if (currentPath == 1 && path1_zajete) {
            currentPath = 2;
            if (path2_zajete) {
                currentPath = 3;
            }
        } else if (currentPath == 2 && path2_zajete) {
            currentPath = 3;
            if (path3_zajete) {
                currentPath = 1;
            }
        } else if (currentPath == 3 && path3_zajete) {
            currentPath = 1;
            if (path1_zajete) {
                currentPath = 2;
            }
        }

        passengerManager.path_number = currentPath;
        cvPaths.notify_all(); // Powiadomienie wątków o zmianie ścieżki
        lock.unlock();
        this_thread::sleep_for(chrono::milliseconds(500)); // Przełączanie ścieżki co 500 ms, jak w poprzenich etapach
    }
}

void updateScreen(PassengerManager& passengerManager, bool& stop_flag) {
    while (!stop_flag || !passengerManager.passengers.empty()) {
        passengerManager.displayPassengers();
        refresh();
        if (stop_flag && passengerManager.passengers.empty()) {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void spaceSTOP(bool& stop_flag) {
    while (!stop_flag) {
        if (getch() == ' ') {
            stop_flag = true;
        }
    }
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    // Inicjalizacja konsoli
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // Inicjalizacja kolor
    start_color(); 
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);

    int screen_width = 20;
    WINDOW* mainwin = newwin(20, 60, 0, 0);
    refresh();
    vector<thread> passenger_threads;

    PassengerManager passengerManager;
    bool stop_flag = false;

    // Wątki i zapobieganie wygłodzaniu
    thread path_thread(selectPath, std::ref(passengerManager), std::ref(stop_flag));
    thread refresh_thread(updateScreen, std::ref(passengerManager), std::ref(stop_flag));
    thread input_thread(spaceSTOP, std::ref(stop_flag));
    thread queue_thread(QueueMonitoring, std::ref(passengerManager), std::ref(stop_flag));

    while (!stop_flag) {
        string name = string(1, generateRandCH());
        double travel_speed = (rand() % 10) / 2.0 + 0.5;

        Passenger passenger;
        passenger.name = name;
        passenger.y_pos = 0;
        passenger.x_pos = 40;
        passenger.travel_speed = travel_speed;
        passenger.QueueStatus = false;
        passengerManager.addPassenger(passenger);

        thread passenger_thread(movePassenger, name, travel_speed, std::ref(passengerManager), screen_width);
        passenger_thread.detach(); //Dzięki temu wątek działa niezależnie od głównego wątku i nie jest zatrzymywany, gdy główny wątek kończy działanie

        double wait_time = (rand() % 20) / 10.0 + 0.5;
        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(wait_time * 2000)));
    }

    for (auto& thread : passenger_threads){
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Joinowanie wątków
    if (path_thread.joinable()) {
        path_thread.join();
    }

    if (refresh_thread.joinable()) {
        refresh_thread.join();
    }

    if (input_thread.joinable()) {
        input_thread.join();
    }

    if (queue_thread.joinable()) {
        queue_thread.join();
    }

    // Zakończenie pracy konsoli
    endwin();

    return 0;
}
