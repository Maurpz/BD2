#include "Page.h"
#include <iostream>

int PageManager::generatebitMapNull (int size) {
  int bitmap = 0;
  for ( int i = 0; i < size; i++) {
    bitmap |= 1 << i;
  }
  return bitmap;
}

void PageManager::changeBitMap(int * bitmap, u_int16_t & index) {
  *bitmap &= ~(1 << (index-1));
}

bool PageManager::isBitNull(uint16_t & bitmap, uint16_t  & index) {
  return (bitmap >> (index-1)) & 1;
}


int calculateTotalSizeOfTuple(vector<Field> & fields) {
  int totalSize = 0;
  totalSize += sizeof(TupleHeader);
  for (Field & column : fields) {
    if (column.refColumn->attlen < 0) {
      totalSize += sizeof(int); //*sumamos el valor que indicara el tamaño del string
      totalSize += column.data.size();
    }
    else {
      totalSize += column.refColumn->attlen;
    }
  }
  return totalSize;
}


uint16_t PageManager::writeTuple(vector<Field> & fields, int & numColumns, int & bitmap, Buffer * page) {
  int totalSize = calculateTotalSizeOfTuple(fields);


  // totalSize += sizeof(TupleHeader);

  // for (Field & column : fields) {
  //   if (column.refColumn->attlen < 0) {
  //     totalSize += sizeof(int); //*sumamos el valor que indicara el tamaño del string
  //     totalSize += column.data.size();
  //   }
  //   else {
  //     totalSize += column.refColumn->attlen;
  //   }
  // }

 //*Preparamos las estrcuturas que usaremos
  int offset = 0;
  Header hp;
  //LinePointer l_ptr;
  TupleHeader th;

  //*recuperamos el headerPage
  memcpy(&hp, page->getData() + offset, sizeof(Header));

  //*calculamos donde deberia de empezar a escribir la tupla
  offset = hp.pd_upper - totalSize;

  //*primero llenamos los datos para el TupleHeader
  th.t_attnum = numColumns; 
  th.t_bits = bitmap;
  th.t_length = totalSize;
  th.t_hoff = offset + sizeof(TupleHeader);

  //*escribimos en el buffer el tupleHeader
  memcpy(page->getData() + offset, &th, sizeof(TupleHeader));
  offset += sizeof(TupleHeader);

  //*procedemos a escribir la data
  for (Field & column : fields) {
    if (column.refColumn->attlen < 0) {
      int size = column.data.size();
      memcpy(page->getData() + offset, &size, sizeof(int));//*escribimos el tamaño
      offset+= sizeof(int);
      memcpy(page->getData() + offset, column.data.c_str(), size);
      offset += size;
    }
    else {
      int num = stoi(column.data);
      memcpy(page->getData() + offset, &num, sizeof(int));
      offset += sizeof(int);
    }
  }

  //*ahora definimos los valores del LinePointer y lo escribimos 
  //l_ptr.length = totalSize;
  //l_ptr.offset = hp.pd_upper - totalSize;

  //*actualizamos el offset
  //offset = hp.pd_lower;

  //memcpy(page->getData() + offset, &l_ptr, sizeof(LinePointer));

  //*finalmente actualizamos los datos del HeaderPage y lo escribimos
  hp.elements++;
  //hp.pd_lower += sizeof(LinePointer);
  hp.pd_upper -= totalSize;

  //*actualizamos el offset aunque tambien podriamos no hacerlo
  memcpy(page->getData(), &hp, sizeof(Header));
  return hp.pd_upper;
}







pair<int, int> PageManager::writeRegisterInBuffer(vector<Field> & fields, int & numColumns, int & bitmap, Buffer * page) {

  Header hp_t;
  memcpy(&hp_t, page->getData(), sizeof(Header));
  cout<<"--------------limites"<<hp_t.pd_lower<<" - "<<hp_t.pd_upper<<endl;
  int espacioDisponible = hp_t.pd_upper-hp_t.pd_lower;
  cout<<"El espacio libre en la apgina actual es de :"<<espacioDisponible<<endl;
  int totalSizeTuple =calculateTotalSizeOfTuple(fields) + sizeof(LinePointer);
  cout<<"El espacio total que necesita el dato es de :"<<totalSizeTuple<<endl;
  if (totalSizeTuple > espacioDisponible) {
    cout<<"????????????????????????NO se puede escribir en esta pagina ya que no hay suficiente espacio"<<endl;
    return make_pair(400, 0); //* Pagina llena
  }


  LinePointer l_ptr;
  l_ptr.offset = writeTuple(fields,numColumns,bitmap,page);
  l_ptr.flags = 1;

  Header hp;
  memcpy(&hp, page->getData(), sizeof(Header));


  int offset = 0;
  offset = hp.pd_lower;
  memcpy(page->getData() + offset, &l_ptr, sizeof(LinePointer));
  hp.pd_lower += sizeof(LinePointer);

  memcpy(page->getData(), &hp, sizeof(Header));
  return make_pair(200, l_ptr.offset);//*200 OK
}


void PageManager::copyFromBuffer(void * destination, void * origin, int & offset, int sizeOf) {
  memcpy(destination,static_cast<char*>(origin) + offset,sizeOf);
  offset += sizeOf;
}

void PageManager::copyToBuffer(void * destination, void * origin, int & offset, int sizeOf) {
  memcpy(static_cast<char*>(destination) + offset, origin, sizeOf);
  //offset += sizeOf;
}