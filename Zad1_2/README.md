# PSI - Laboratorium Zadanie 1.2 (Niezawodna transmisja UDP)

## Zespół nr 39

**Temat:** Implementacja niezawodnej transmisji danych typu
*Stop-and-Wait* nad protokołem UDP.\
**Klient:** C\
**Serwer:** Python

## Opis Projektu

Projekt realizuje niezawodną transmisję danych przy użyciu protokołu UDP
oraz własnego protokołu aplikacyjnego opartego na mechanizmie
*Stop-and-Wait*.\
Celem zadania jest przesłanie pliku binarnego o rozmiarze **10 000
bajtów** w 100 pakietach po 100 bajtów.

### Serwer (Python)

-   Nasłuchuje na porcie **8000**.
-   Odbiera pakiety i rekonstruuje cały plik.
-   Weryfikuje numer sekwencyjny i wysyła **ACK**.
-   Obsługuje duplikaty pakietów.
-   Po zakończeniu transmisji oblicza hash **DJB2** (`SERVER HASH`).

### Klient (C)

-   Wczytuje `random.bin`.
-   Dzieli dane na 100 fragmentów po 100 bajtów.
-   Wysyła kolejne pakiety i oczekuje **ACK** (timeout 2 sekundy).
-   Retransmituje pakiety w razie braku odpowiedzi.
-   Przed wysyłką oblicza hash **DJB2** (`CLIENT HASH`).

Dodatkowo projekt zawiera testy z wykorzystaniem `tc`, które symulują
straty pakietów (30% oraz 60%).

## Struktura Repozytorium
* `Server/` - Kod źródłowy serwera (`server.py`) oraz `Dockerfile`.
* `Client/` - Kod źródłowy klienta (`client.c`) oraz `Dockerfile` i skrypt do generowania pliku wejściowego `generate_file.sh`.
* `start.sh` - Skrypt automatyzujący budowę obrazów i uruchomienie podstawowego testu.
* `test_loss.sh` - Skrypt uruchamiający testy z utratą pakietów.

## Wymagania

-   Docker
-   Środowisko bigubu

## Uruchomienie

### 1. Podstawowy test transmisji

    chmod +x start.sh
    ./start.sh

### 2. Testy z utratą pakietów (30% i 60%)

    chmod +x test_loss.sh
    ./test_loss.sh