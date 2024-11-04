#include "Catalog.h"

Catalog::Catalog() {
  char name [] = {'h','o','l','a',0};
  types["INT"] = make_unique<PgType>(121, name, 4, 'N');

  r_and_w = make_unique<fstream>();
}

bool Catalog::createTable(string tableName) {
  PgClassRow tempClassRow;
  int oid = 2223;
  tempClassRow.oid = oid;

  //asignamos el nombre de la tabla
  int i = 0;
  for(i;i<tableName.length();i++){
    tempClassRow.relname[i] = tableName[i];
  }
  tempClassRow.relname[i] = 0;
  //---------------------

  tempClassRow.relam = 1111;
  tempClassRow.relpages = 1;
  tempClassRow.reltuples = 0;
  tempClassRow.relkind = 'r';
  tempClassRow.relfilenode = oid;

  string dataBaseName = "";
  string dirName = "";

  r_and_w->open(dataBaseName+dirName, ios::app | ios::binary | ios::out | ios::in);
  if (! r_and_w->is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return false;
  }
  r_and_w->write(reinterpret_cast<char*>(&tempClassRow),sizeof(PgClassRow));
  r_and_w->close();
  return true;
}

void Catalog::getOidByTableName(string tableName) {
  char tempNameArray[MAX_SIZE_CHAR];
  int lengthName = tableName.length();
  strncpy(tempNameArray,tableName.c_str(),lengthName+1);
  tempNameArray[lengthName] = 0;

  string dataBaseName = "";
  string dirName = "";

  r_and_w->open(dataBaseName+dirName, ios::app | ios::binary | ios::out | ios::in);
  
  PgClassRow temp;
  while(r_and_w->read(reinterpret_cast<char*>(&temp),sizeof(PgClassRow))) {
    cout << "OID: " << temp.oid <<endl;
    cout << "Nombre: " << temp.relname <<endl;
    cout << "Páginas: " << temp.relpages <<endl;
    cout << "Tuplas: " << temp.reltuples <<endl;
    cout << "--------------------------" <<endl;
  }
  r_and_w->close();

}