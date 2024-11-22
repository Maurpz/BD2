#include <cstdint>
#include "../catalog/Catalog.h"
#include "../buffer/BufferManager.h"

#ifndef PAGE_H
#define PAGE_H

struct Header {
  uint32_t elements;  //indica el numero de elementos que hay en una pagina
  uint16_t pd_lower;  //indica el limite inferios del espacio libre
  uint16_t pd_upper;  //indica el limite superios del espacio libre
};

//line pointer de cada registro old
/*
struct LinePointer {
  uint16_t offset;  //indica donde comienza el registro
  uint16_t length;  //indica el tamaño del registro
};
*/
struct LinePointer {
  uint16_t offset;  //indica donde comienza el registro
  uint16_t length;  //indica el tamaño del registro
  uint16_t flags;
};

enum LinePointerFlags {
  LP_NORMAL = 1,
  LP_DEAD = 2
};

/*
struct TupleHeader {
  uint16_t t_length;  //tamaño de total del header y la data
  uint16_t t_hoff;    //indica el inicio de los datos reales (tuple data)
  uint16_t t_bits;    //bitmap de valores nulos
  uint16_t t_attnum;  //numero de atributos
};
*/
struct TupleHeader {
  uint16_t t_length;  //tamaño de total del header y la data
  uint16_t t_hoff;    //indica el inicio de los datos reales (tuple data)
  uint16_t t_bits;    //bitmap de valores nulos
  uint16_t t_attnum;  //numero de atributos
  uint16_t flags;
};

enum TupleHeaderFlags {
  TH_NORMAL = 1,
  TH_UPDATED = 2,
  TH_DELETED = 3,
};


class PageManager {
  private:

  public:
    PageManager();

    static int generatebitMapNull(int size);
    static void changeBitMap(int * bitmap, u_int16_t & index);
    static bool isBitNull(uint16_t & bitmap, uint16_t  & index);
    static void copyFromBuffer(void * destination, void * origin, int & offset, int sizeOf);
    static void copyToBuffer(void * destination, void * origin, int & offset, int sizeOf);

    //static void 
    static void writeRegisterInBuffer(vector<Field> & fields, int & numColumns, int & bitmap, Buffer * page);
    static uint16_t writeTuple(vector<Field> & fields, int & numColumns, int & bitmap, Buffer * page);

    
};

#endif