#include "./Disck.hpp"
#include <memory>

#ifndef DISCK_MANAGER_HPP
#define DISCK_MANAGER_HPP
struct HeaderInfoFile {
  int offset;
  int numFiles;
};

struct InfoFile {
  char nameFile[25];
  int numBlock;
  int sizeData;
};

class DisckManager {
  private:
    unique_ptr<Disck> disck;

  public:
    DisckManager(int numeroDePlatos, int numeroDePistas, int numeroDeBloques, int numeroDeSectores, int sizeSector,int blocksReservados);
    ~DisckManager();
    void saveFile(string nameFile, char * data, int size);
    void updateFile(int numBlock, char * data, int size);
    void indexingFile(string nameFile, int numBlock, int dataSize);
    void createDirectories();
    pair<int, int> findFile(string name);//*primero retorna el numero de bloque y segundo el tamaño  
    unique_ptr<char[]> getBlockByNumber(int numBlock, int sizeData);

};
#endif