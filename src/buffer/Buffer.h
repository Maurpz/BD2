#ifndef BUFFER_H
#define BUFFER_H


//const int PAGE_SIZE = 4096;
//const int PAGE_SIZE = 4096;
//const int PAGE_SIZE = 512;
//const int PAGE_SIZE = 206;
const int PAGE_SIZE = 50;



class Buffer {
  private:
    //char * data;
    unique_ptr<char []> data;
    int fileNodeOID;
    int pageNum;
    bool dirty;
    int pinCount;
  public:
    Buffer(int fileNodeOID, int pageNum);
    ~Buffer();

    char * getData();
    int getFileNodeOID();
    void setIsDirty(bool);
    bool isDirty();
    void setData(unique_ptr<char []> data);//!posiblemente eliminar
};
#endif