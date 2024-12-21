#ifndef DISCK_HPP
#define DISCK_HPP

#include <filesystem>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
using namespace std;

namespace fs = std::filesystem; 
class Disck {
  private :
    string root;

    int numeroDePlatos;
    int pistasPorSuperficie;
    int bloquesPorPista;
    int sectoresPorBloque;
    int bytesPorSector;

    int numeroTotalDeBloques;
    int bloquesPorPlato;
    int bloquesPorSuperficie;
    int bytesPorBloque;

    int blocksReservados;

  public :
    Disck(int numeroDePlatos, int numeroDePistas, int numeroDeBloques, int numeroDeSectores, int sizeSector, int blocksReservados);
    ~Disck();
    void createDirectories();

    int getFreeBlock(int size);

    pair<string,int> ubicacionDelBloque(int index);
    void writeToDisck(int numBloque, char * data, int size);

    void updateStateBlock(int numBlock);

};
#endif