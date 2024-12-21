

using namespace std;
#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H
#include "Buffer.h"
#include "../disck/DisckManager.hpp"
#include "../btree/bplustree.cpp"
#include <map>
#include <unordered_map>
#include <fstream>
#include <memory>
#include <cstring>
#include <functional> // Para std::hash

struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        // Combina los hashes de ambos elementos
        return std::hash<T1>()(pair.first) ^ (std::hash<T2>()(pair.second) << 1);
    }
};


struct PageTest {
  int day;
  int year;
};

class BufferManager {
  private:
    map<pair<int,int>,unique_ptr<Buffer>> bufferPool;
    //unordered_map<pair<int, int>, list<pair<int, int>>::iterator> lruMap; // Mapa para acceso rápido
    unordered_map<pair<int, int>, list<pair<int, int>>::iterator, PairHash> lruMap;
    list<pair<int, int>> lruList; // Lista para rastrear el orden de uso (LRU)
    //unique_ptr <fstream> r_and_w;
    DisckManager * diskmg;
    int maxSize;
    // string nameDB;
    // string host;
    
    //cf
    vector<pair<int,unique_ptr<BPlusTree>>> trees;
    
  public:
    BufferManager(DisckManager * diskmg, int maxSize_);
    ~BufferManager();

    Buffer * loadPage(uint32_t fileNodeOID, int pageNum);

    //sincroniza la pagina del buffer con el archivo en disco
    void flushPage(int fileNodeOID, int pageNum);

    //crear nueva pagina
    Buffer * newPage(int fileNodeOID, int pageNum);
    
    //marca para que la pagina no sea expulsada 
    //inmediatamente usa las politicas de remplazo
    //void pinPage(int fileNodeOID, int pageNum);

    //desmarca la pagina para que pueda ser elimnada si le toca
    //void unPinPage(int fileNodeOID, int pageNum);
    
    //expulsa la pagina del buffer para liberar espacio seleccionando
    //la pagina segun la politica de remplazo
    void evictPage();

    //bool addPage(int fileNodeOID);
    bool isPageInBuffer(pair<int,int> key);

    //marca una pagina com sucia para indicar que fue modificada
    //y debe ser escrita en el disco antes de ser expulsada
    //void markPageDirty(int fileNodeOID, int pageNum);

    //libera todas las paginas escribiendolas sucias si es necesario
    //void releaseAllPages();


    //indices
    void addBTree(int indexOID);
    BPlusTree * getBTree(int indexOID);


    //funciones de complemento
    //void setDbName(string nameDB);//*no se usara
    //void setHostName(string host);//*no se usara
    //void printStatus();//*no se usara

};
#endif