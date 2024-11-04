#include "BufferManager.h"
#include <iostream>
using namespace std;

BufferManager::BufferManager(){
  r_and_w = make_unique<fstream>();
}

bool BufferManager::existsPage(pair<int,int> key) {
  if (bufferPool.find(key) != bufferPool.end()) return true;
  return false;
}

Buffer * BufferManager::getPage(int fileNodeOID, int pageNum) {
  pair<int,int> key = make_pair(fileNodeOID,pageNum);
  if (existsPage(key)) return bufferPool[key].get();

  string dirName ="";
  this->r_and_w->open(dirName,ios::binary | ios::out | ios::in);

  if (! r_and_w->is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return;
  }

  unique_ptr<Buffer> temp = make_unique<Buffer>(fileNodeOID,pageNum);
  int offset = PAGE_SIZE * pageNum;
  r_and_w->seekg(offset,ios::beg);

  if(!r_and_w->read(temp->getData(),PAGE_SIZE)) {
    throw runtime_error("No se pudo leer la pagina");
  }
  r_and_w->close();

  bufferPool[key] = move(temp);
  cout<<"Operacion completada con exito"<<endl;
  return bufferPool[key].get();

}