#include "Executor.h"
#include <vector>
#include "../storage/Page.h"
using namespace std;

Executor::Executor(){}

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

  //todo podriamos implementar un diccionario cuando busque las columna asi no buscamos por iteracion
  Buffer * page = refBufferManager->loadPage(table->oid,0);

  Field field;
  field.refColumn = column.get();
  field.data = data;
  field.isNull = false;

  vector<Field> fields;
  fields.push_back(field);


  int bitmapNull = 0;
  for (size_t i = 0; i < fields.size(); i++) {
    if (fields[i].isNull) {
      bitmapNull |= 1 << i;
    }
  }

  //*creamos un offset para el buffer
  //*despues podriamos añadirlo a la misma clase buffer quisa
  int offset = 0;

  //*escribimos en la pagina primero leyendo su contenido
  Header headPage;
  memcpy(&headPage,page->getData(),sizeof(Header));
  offset = headPage.pd_lower;
  cout<<"Recuperamos el header del pagina en el buffer"<<endl;
  cout<<"pd_lower : "<<headPage.pd_lower<<endl;
  cout<<"pd_upper : "<<headPage.pd_upper<<endl;
  cout<<"numero de tuplas : "<<headPage.elements<<endl;

  //*creamos el line pointer
  LinePointer pointer;
  int dataSize = 0;
  for (size_t i = 0; i < fields.size(); i++) {
    if (!fields[i].isNull) {
      if (field.refColumn->attlen>0) {
        dataSize +=fields[i].refColumn->attlen;
      }
      //* para calcular el tamaño total sin del tupla
      // else {
      //   dataSize+=sizeof(int);
      //   dataSize+=fields[i].data.
      // }
    }
  }
  dataSize+= sizeof(TupleHeader);

  cout<<"El tamaño de la data que se insertara es de :"<<dataSize<<endl;

  //*creamos el tuple header
  TupleHeader tHeader;
  tHeader.t_attnum = fields.size();
  tHeader.t_length = dataSize;
  tHeader.t_bits = bitmapNull;

  //*posicionamos el cabezal para la escritura
  pointer.offset = headPage.pd_upper - dataSize;
  pointer.length = dataSize;
  offset = pointer.offset;

  tHeader.t_hoff =offset + sizeof(TupleHeader);

  memcpy(page + offset, &tHeader, sizeof(TupleHeader));

  //*actualizamos el offset con la nueva escritura
  offset += sizeof(TupleHeader);

  memcpy(page + offset, &data, sizeof(int));
  offset += sizeof(int);


  //*termino de escribir la tupla ahora escribiremos el linePointer
  offset = headPage.pd_lower;
  memcpy(page + offset, &pointer, sizeof(LinePointer));
  offset+= sizeof(LinePointer);

  //*final mente actualizamos los valores de PageHeader
  headPage.pd_upper -= dataSize;
  headPage.pd_lower += sizeof(LinePointer);
  headPage.elements++;

  offset = 0;
  memcpy(page + offset, &headPage, sizeof(Header));

  page->setIsDirty(true);

  //todo: verificar que mas se debe de ahcer con el buffer marcar com sucio y que mas procede ?


}

// int createBitMapNull(vector<Field> & fields){
//   int bitmapNull = 0;
//   for (size_t i = 0; i < fields.size(); i++) {
//     if (fields[i].isNull) {
//       bitmapNull |= 1 << i;
//     }
//   }
//   return bitmapNull;
// }