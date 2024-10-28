#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#ifndef STRUCTS_H 
#define STRUCTS_H

// Tamaño de la página (8 KB)
const int PAGE_SIZE = 8192;

// Estructura del encabezado de la página
struct PageHeader {
    uint32_t pd_lsn;       // Log Sequence Number
    uint16_t pd_lower;     // Inicio del espacio libre
    uint16_t pd_upper;     // Fin del espacio libre
};
struct LinePointer {
  uint16_t offset;
  uint16_t length;
};
#endif