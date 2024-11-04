#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include "Strucs.h"



void readHeaderFromFile(const std::string& filename) {
    // Abrir el archivo binario en modo lectura
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo para lectura" << std::endl;
        return;
    }

    // Crear una instancia de PageHeader para almacenar los datos leídos
    PageHeader header;

    // Leer el header desde el archivo
    file.read(reinterpret_cast<char*>(&header), sizeof(PageHeader));

    // Cerrar el archivo
    file.close();

    // Mostrar los datos leídos
    std::cout << "Datos del Header:" << std::endl;
    std::cout << "pd_lsn: " << header.pd_lsn << std::endl;
    std::cout << "pd_lower: " << header.pd_lower << std::endl;
    std::cout << "pd_upper: " << header.pd_upper << std::endl;
}

int main() {
    // Ruta del archivo binario que deseas leer

    // Leer y mostrar el encabezado de la página
    readHeaderFromFile("./bin/02.bin");

    return 0;
}
