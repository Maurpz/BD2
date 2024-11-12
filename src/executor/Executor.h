


#include "../catalog/Catalog.h"
#include "../buffer/BufferManager.h"
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
    //*metodos para 1 sola columnas (longitud fija)
    void executeInsert1(string nameTable, string nameColumn, int data);
    void executeSelectAll1(string nameTable);

    //*metodos para 2 columnas (longitud fija)
    void executeInsert2(string nameTable, string nameColumn, int data, string column2, int data2);
    void executeSelectAll2(string nameTable);

    void executeInsertString(string nameTable, string nameColumn, string data);
    void executeSelectAllStrings (string nameTable);

    //* select para all para caolumnas de diferente tipo
    void insertInto(string nameTable, vector<Col_Data> & rawData);
    void selectAll(string nameTable);

    void selectCustom(string nameTable, vector<string> & fields);
};
#endif