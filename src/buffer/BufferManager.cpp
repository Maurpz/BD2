#include "BufferManager.h"
#include <iostream>
#include "storage/Page.h"
using namespace std;

BufferManager::BufferManager(){
  r_and_w = make_unique<fstream>();
}

bool BufferManager::isPageInBuffer(pair<int,int> key) {
  if (bufferPool.find(key) != bufferPool.end()) return true;
  return false;
}

Buffer * BufferManager::loadPage(int fileNodeOID, int pageNum) {
  // pair<int,int> key = make_pair(fileNodeOID,pageNum);
  // if (isPageInBuffer(key)) return bufferPool[key].get();

  // string dirName ="";
  // this->r_and_w->open(dirName,ios::binary | ios::out | ios::in);

  // if (! r_and_w->is_open()) {
  //   cout<<"NO se pudo abrir"<<endl;
  //   return nullptr;
  // }

  // unique_ptr<Buffer> temp = make_unique<Buffer>(fileNodeOID,pageNum);
  // int offset = PAGE_SIZE * pageNum;
  // r_and_w->seekg(offset,ios::beg);

  // if(!r_and_w->read(temp->getData(),PAGE_SIZE)) {
  //   throw runtime_error("No se pudo leer la pagina");
  // }
  // r_and_w->close();

  // bufferPool[key] = move(temp);
  // cout<<"Operacion completada con exito"<<endl;
  // return bufferPool[key].get();
  pair<int,int> key = make_pair(fileNodeOID,pageNum);
  if (isPageInBuffer(key)) return bufferPool[key].get();
  string slash = "/";
  
  string dirName = host+slash+"data"+slash+nameDB+slash+to_string(fileNodeOID)+".bin";



}



// Buffer * BufferManager::newPage(int fileNodeOID) {
//   string dirName ="testBufferM.bin";
//   r_and_w->open(dirName, ios::binary | ios::in);
//   int tam = 0;
//   if(r_and_w->is_open()) {
//     r_and_w->seekg(0,ios::end);
//     tam = r_and_w->tellg() / PAGE_SIZE;
//     cout<<"tam: "<<tam<<endl;
//   }


  
//   char memtemp[PAGE_SIZE];
//   for (int i = 0; i<tam; i++) {
//     r_and_w->seekg(PAGE_SIZE* i,ios::beg);
//     cout<<"------------------"<<endl;
//     r_and_w->read(memtemp,PAGE_SIZE);
//     PageTest t11;
//     memcpy(&t11,memtemp,sizeof(PageTest));
//     cout<<"Contenido: "<<t11.day<<" "<<t11.year<<endl;
//   }
//   r_and_w->close();

//   return nullptr;
// }

Buffer * BufferManager::newPage(int fileNodeOID) {
  // string dirname = "testBufferM.bin";
  // char page[PAGE_SIZE];
  // memset(page,0,sizeof(page));

  // PageTest t1;
  // t1.day = day;
  // t1.year = year;
  // memcpy(page, &t1, sizeof(PageTest));
  // cout<<"se escribe en ela rchivo"<<endl;
  // r_and_w->open(dirname, ios::binary | ios::app | ios::out);
  // r_and_w->write(page,PAGE_SIZE);
  // r_and_w->close();
  // return true;
  string slash = "/";
  string dirName = host+slash+"data"+slash+nameDB+slash+to_string(fileNodeOID)+".bin";
  r_and_w->open(dirName,ios::binary | ios::in | ios::out);
  r_and_w->seekg(0,ios::end);
  int pageNum = (r_and_w->tellg() / PAGE_SIZE) - 1;


  //inicializacion de la pageHeader
  Header newHeaderPage;
  newHeaderPage.elements = 0;
  newHeaderPage.pd_lower = PAGE_SIZE-sizeof(Header);
  newHeaderPage.pd_upper = PAGE_SIZE;

  //creamos el buffer y le agregamos su pageHeader
  unique_ptr<Buffer> newPageTemp = make_unique<Buffer>(fileNodeOID, pageNum);
  memcpy(newPageTemp->getData(),&newHeaderPage,sizeof(Header));

  //escribimos este buffer a disco para tenerlo sincronizado
  r_and_w->write(newPageTemp->getData(),PAGE_SIZE);
  r_and_w->close();

  //agregamos el buffer el bufferPool
  pair<int,int> key = make_pair(fileNodeOID, pageNum);
  this->bufferPool[key] = move(newPageTemp);

  //retorname el un puntero al buffer previamente creado y guardado
  return bufferPool[key].get();
}

void BufferManager::setDbName(string name) {
  this->nameDB = name;
}

void BufferManager::setHostName(string host) {
  this->host = host;
}
