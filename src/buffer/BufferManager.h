
using namespace std;

#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H
#include "Buffer.h"
#include <map>
#include <fstream>
#include <memory>
#include <cstring>

struct PageTest {
  int day;
  int year;
};

class BufferManager {
  private:
    map<pair<int,int>,unique_ptr<Buffer>> bufferPool;
    unique_ptr <fstream> r_and_w;
    int maxSize;
    string nameDB;
    string host;
    
  public:
    BufferManager();
    Buffer * loadPage(int fileNodeOID, int pageNum);

    //sincroniza la pagina del buffer con el archivo en disco
    void flushPage(int fileNodeOID, int pageNum);

    //crear nueva pagina
    Buffer * newPage(int fileNodeOID);
    
    //marca para que la pagina no sea expulsada 
    //inmediatamente usa las politicas de remplazo
    void pinPage(int fileNodeOID, int pageNum);

    //desmarca la pagina para que pueda ser elimnada si le toca
    void unPinPage(int fileNodeOID, int pageNum);
    
    //expulsa la pagina del buffer para liberar espacio seleccionando
    //la pagina segun la politica de remplazo
    void evictPage();

    bool addPage(int fileNodeOID);
    bool isPageInBuffer(pair<int,int> key);

    //marca una pagina com sucia para indicar que fue modificada
    //y debe ser escrita en el disco antes de ser expulsada
    void markPageDirty(int fileNodeOID, int pageNum);

    //libera todas las paginas escribiendolas sucias si es necesario
    void releaseAllPages();


    //funciones de complemento
    void setDbName(string nameDB);
    void setHostName(string host);
    void printStatus();

};
#endif