#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <ncurses.h>

using namespace std;

struct Passenger {
    string name;
    int x_pos;
    int y_pos;
    double travel_speed;
};

char generateRandCH() {
    const char charset[] = "!@#$%^&*?ABCDEFGHIJKLMOPRSTUWXYZQ0123456789abcdefghijklmnoprstuwxyzq";
    const int max_index = sizeof(charset) - 1;
    return charset[rand() % max_index];
}

struct PassengerManager {
    vector<Passenger> passengers;
    int path_number;

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
        mvprintw(11, 19, "$"); mvprintw(11, 21, "$");
        mvprintw(10, 39, "x"); mvprintw(10, 41, "x");
        mvprintw(10, 59, "x"); mvprintw(10, 61, "x");
        }
        else if (path_nr == 2) {
        mvprintw(10, 19, "x"); mvprintw(10, 21, "x");
        mvprintw(11, 39, "$"); mvprintw(11, 41, "$");
        mvprintw(10, 59, "x"); mvprintw(10, 61, "x");
        }
        else if (path_nr == 3) {
        mvprintw(10, 19, "x"); mvprintw(10, 21, "x");
        mvprintw(10, 39, "x"); mvprintw(10, 41, "x");
        mvprintw(11, 59, "$"); mvprintw(11, 61, "$");
        }
    
        mvprintw(10, 1, "DYSTRYBUTOR");

        mvprintw(16, 19, "(---)"); 
        mvprintw(16, 39, "(---)");
        mvprintw(16, 59, "(---)");

        mvprintw(16, 1, "STANOWISKA");


        refresh();
    }
};

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

            if (passengerPtr->y_pos == 10) {
                if (passengerManager.path_number == 1) {
                    passengerPtr->x_pos -= 20;
                    passengerPtr->y_pos++;
                }
                else if (passengerManager.path_number == 3) {
                    passengerPtr->x_pos += 20;
                    passengerPtr->y_pos++;
                }
            }

            if (passengerPtr->y_pos == 15) {
                this_thread::sleep_for(chrono::seconds(2));
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

//Funkcja zapobiegająca wygłodzeniu wątków
void addPassengerAndStartThread(string name, double travel_speed, PassengerManager& passengerManager, int screen_width) {
    Passenger passenger;
    passenger.name = name;
    passenger.y_pos = 0;
    passenger.x_pos = 40;
    passenger.travel_speed = travel_speed;
    passengerManager.addPassenger(passenger);

    thread passenger_thread(movePassenger, name, travel_speed, std::ref(passengerManager), screen_width);
    passenger_thread.detach(); //Dzięki temu wątek działa niezależnie od głównego wątku i nie jest zatrzymywany, gdy główny wątek kończy działanie

    double wait_time = (rand() % 20) / 10.0 + 0.5;
    this_thread::sleep_for(chrono::milliseconds(static_cast<int>(wait_time * 2000)));
}

// Kontroluje wykonanie pętli zapewniając, że wątki te będą działać dopóki flaga stop_flag nie zostanie ustawiona na true.
// Dzięki temu wątki te nie zostaną zatrzymane przed zakończeniem ich pracy
void selectPath(PassengerManager& passengerManager, bool& stop_flag) {
    while (!stop_flag || !passengerManager.passengers.empty()) { 
        auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now().time_since_epoch()).count();
        passengerManager.path_number = (elapsed_seconds / 2) % 3 + 1;
        this_thread::sleep_for(chrono::milliseconds(500));
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

    PassengerManager passengerManager;
    volatile bool stop_flag = false;

    // Wątki i zapobieganie wygłodzaniu
    thread path_thread(selectPath, std::ref(passengerManager), std::ref(stop_flag));
    thread refresh_thread(updateScreen, std::ref(passengerManager), std::ref(stop_flag));
    thread input_thread(spaceSTOP, std::ref(stop_flag));

    while (!stop_flag) {
        string name = string(1, generateRandCH());
        double travel_speed = (rand() % 10) / 2.0 + 0.5;

        addPassengerAndStartThread(name, travel_speed, passengerManager, screen_width);
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

    // Zakończenie pracy konsoli
    endwin();

    return 0;
}
