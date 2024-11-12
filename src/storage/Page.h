#include <cstdint>


#ifndef PAGE_H
#define PAGE_H

struct Header {
  uint32_t elements;  //indica el numero de elementos que hay en una pagina
  uint16_t pd_lower;  //indica el limite inferios del espacio libre
  uint16_t pd_upper;  //indica el limite superios del espacio libre
};

//line pointer de cada registro
struct LinePointer {
  uint16_t offset;  //indica donde comienza el registro
  uint16_t length;  //indica el tamaño del registro
};

struct TupleHeader {
  uint16_t t_length;  //tamaño de total del header y la data
  uint16_t t_hoff;    //indica el inicio de los datos reales (tuple data)
  uint16_t t_bits;    //bitmap de valores nulos
  uint16_t t_attnum;  //numero de atributos
};


class PageManager {
  private:

  public:
    PageManager();

    static int generatebitMapNull(int size);
    static void changeBitMap(int * bitmap, u_int16_t& index);
    static bool isBitNull(uint16_t & bitmap, uint16_t  & index);

    
};

#endif