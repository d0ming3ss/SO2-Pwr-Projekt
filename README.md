Temat projektu: Klienci i dystrybutor.

Celem projektu jest stworzenie programu wielowątkowego w języku C++ z wykorzystaniem odpowiednich bibliotek graficznych.
W moim projekcie wykorzystałem bibliotekę Ncurses.
Projekt składa się z 3 etapów, gdzie 1 etap jest ogólny dla każdego, a następne etapy projektu są przydzielanie indywidualnie dla każdego studenta.

Cel faktycznego projektu:

Projekt i realizacja obsługi dystrybutora dla "klientów". Dystrybutor ma za zadanie przełączać się czasowo między trzema dostepnymi stanowiskami końcowymi.
Klienci poruszają się losową prędkością.

Jak to działa?

1. Klienci docierają do dystrybutora.
2. Dystrybutor przełacza się czasowo między trzema stanowiskami.
3. Klient zostaje przydzielony poprzez dystrybutor do jednego z trzech stanowisk końcowych.
4. Gdy klienci dotrą do stanowiska końcowego, ich cykl "życia" się kończy.
5. Zatrzymanie programu jest możliwe po naciśnięciu SPACJI.


Zapamiętać!
Każda grupa projektowa ma INNY TEMAT PROJEKTU!!

main2.cpp -> ETAP 1.

main2.2.cpp -> ETAP 2. 
Założenia do II etapu:

Klienci czekają w kolejce "polskiej" przed dystrybutorem i wchodzą pojedyńczo. Tylko jeden klient wewnatrz dystrybutora i to samo do stanowisk.

main3.cpp -> ETAP 3.
Założenia do III etapu:

Korekta poprzedniego programu, oczekujący w kolejce przed dystrybutorem może wybrać inne nie zajęte stanowisko końcowe.
