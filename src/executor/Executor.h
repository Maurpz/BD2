
#include "../buffer/BufferManager.h"
#include "../catalog/Catalog.h"


#ifndef EXECUTOR_H
#define EXECUTOR_H

class Executor {
  private:
  BufferManager * refBufferManager;
  Catalog * refCatalog;

  public:
    Executor();

    void setReferenceToBufferManager(BufferManager * referenceBM);
    void setReferenceToCatalog(Catalog * referenceCat);

    //*consultas basicas
    void executeInsert();
    void executeSelect();

    //metodos test
    void executeInsert1(string nameTable, string nameColumn, int data);
};
#endif