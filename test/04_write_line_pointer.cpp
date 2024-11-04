#include <iostream>
#include <fstream>
#include <cstring>
#include "Strucs.h"

using namespace std;

void readHeaderFromFile(const string& filename) {
    // Abrir el archivo binario en modo lectura
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "No se pudo abrir el archivo para lectura" << endl;
        return;
    }

    // Crear una instancia de PageHeader para almacenar los datos leídos
    PageHeader header;

    // Leer el header desde el archivo
    file.read(reinterpret_cast<char*>(&header), sizeof(PageHeader));

    // Cerrar el archivo
    file.close();

    // Mostrar los datos leídos
    cout << "Datos del Header:" << endl;
    cout << "pd_lsn: " << header.pd_lsn << endl;
    cout << "pd_lower: " << header.pd_lower << endl;
    cout << "pd_upper: " << header.pd_upper << endl;
}

int main() {
    // Ruta del archivo binario que deseas leer

    // Leer y mostrar el encabezado de la página
    readHeaderFromFile("./bin/02.bin");

    return 0;
}
