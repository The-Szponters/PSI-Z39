# PSI - Laboratorium Zadanie 2 (TCP)

## Zespół nr 39
**Temat:** Serwer współbieżny TCP (C) oraz Klient (Python). Obliczanie skrótu wiadomości (Hash DJB2).

## Opis Projektu
Projekt realizuje architekturę klient-serwer przy użyciu protokołu TCP w środowisku Docker.
* **Serwer (C):** Implementuje obsługę wielu klientów jednocześnie (współbieżność) przy użyciu mechanizmu `fork()`. Serwer oblicza skrót (hash) otrzymanej wiadomości algorytmem DJB2.
* **Klient (Python):** Łączy się z serwerem, wysyła wiadomość tekstową i weryfikuje poprawność otrzymanego skrótu. Klient posiada wbudowany mechanizm symulacji opóźnienia (`sleep`), co pozwala udowodnić równoległe przetwarzanie żądań przez serwer.

## Struktura Repozytorium
* `Server/` - Kod źródłowy serwera (`server.c`) oraz `Dockerfile`.
* `Client/` - Kod źródłowy klienta (`client.py`) oraz `Dockerfile`.
* `start.sh` - Skrypt automatyzujący budowę obrazów i uruchomienie testów.

## Wymagania
* Docker
* Środowisko bigubu

## Uruchomienie
Aby zbudować obrazy i uruchomić test, należy wykonać skrypt `start.sh`.
 ```bash
 chmod +x start.sh
./start.sh
```
