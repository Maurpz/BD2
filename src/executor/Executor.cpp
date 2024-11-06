#include "Executor.h"


void Executor::setReferenceToBufferManager(BufferManager * referenceBM) {
  this->refBufferManager = referenceBM;
}

void Executor::setReferenceToCatalog(Catalog * referenceCat) {
  this->refCatalog = referenceCat;
}

//*insert para 1 sola columna
void Executor::executeInsert1(string nameTable, string nameColumn, int data) {
  //todo: falta hacer comprovaciones de la existencia de tablas y columnas 
  //todo: falta hacer validadciones de tipos a datos que se insertaran 
  //todo: saneamiento de datos
  //!estamos suponiendo que sabemos que existe la tabla
  unique_ptr<PgClassRow> table =refCatalog->getTable(nameTable);
  unique_ptr<PgAttributeRow> column = refCatalog->getColumn(table->oid,nameColumn);

  //*pedir al buffer manager la pagina
  //*por defecto pido la primera pagina pero suponog que seria la ultima
  //*para poder escribir consultar eso
  //* por el momento solo trabajamos con el mejor de los casos
  Buffer * page = refBufferManager->loadPage(table->oid,0);

}