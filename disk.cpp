#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#define SECTOR_SIZE 256
#define SECTOR_COUNT 16
#define VTOC_OFFSET SECTOR_COUNT * 17 * SECTOR_SIZE

using namespace std;

class Disk {

    fstream disk;

    Disk() {
        disk.open("copy.dsk", ios_base::binary | ios_base::in);
    }

    ~Disk() {
        disk.close();
    }

    Disk(const Disk&) = delete;

    Disk& operator=(const Disk&) = delete;

public:

    static int read(int size) {
        int res = 0;
        for (int i = 0; i < size; ++i) {
            res = res << 8;
            res |= getDisk().get();
        }
        return res;
    }

    static fstream& getDisk() {
        static Disk _this;
        return _this.disk;
    }
};

class File {

public:
    int flFIags;
    int flTyp;
    int flUsrWds;
    int flFINum;
    int flStBlk;
    int flLgLen;
    int flPyLen;
    int flRStBlk;
    int flRLgLen;
    int flRPyLen;
    int flCrDat;
    int flMdDat;
    int flNam;
    string flNamS;

    int fBegin;
    int fEnd;

    File(int offset) {
        Disk::getDisk().seekg(offset);
        flFIags  = Disk::read(1);
        if (isValid()) {
            flTyp    = Disk::read(1);
            flUsrWds = Disk::read(16);
            flFINum  = Disk::read(4);
            flStBlk  = Disk::read(2);
            flLgLen  = Disk::read(4);
            flPyLen  = Disk::read(4);
            flRStBlk = Disk::read(2);
            flRLgLen = Disk::read(4);
            flRPyLen = Disk::read(4);
            flCrDat  = Disk::read(4);
            flMdDat  = Disk::read(4);
            flNam    = Disk::read(1);

            char *temp = new char[flNam + 1];
            Disk::getDisk().get(temp, flNam + 1);
            flNamS = string(temp);
            delete temp;

            fBegin = offset;
            fEnd   = fBegin + 51 + flNam;
            if (fEnd % 2) {
                ++fEnd;
            }
            printAll();
        }
    }

    bool isValid() {
        /*return flFIags & 0x80;*/
        return flFIags == 0x80;
    }

    void printAll() {
        cout << "--------------------" << endl;
        cout << hex;
        cout << flFIags  << endl;
        cout << flTyp    << endl;
        cout << flUsrWds << endl;
        cout << flFINum  << endl;
        cout << flStBlk  << endl;
        cout << flLgLen  << endl;
        cout << flPyLen  << endl;
        cout << flRStBlk << endl;
        cout << flRLgLen << endl;
        cout << flRPyLen << endl;
        cout << flCrDat  << endl;
        cout << flMdDat  << endl;
        cout << flNam    << endl;
        printFileName();
        cout << "--------------------" << endl;
    }

    void printFileName() {
        cout << "\\" << flNamS << "\\" << endl;
    }

};

int main() {  
    int fBegin = 128 * 16;
    /*for(;;) {  
        File f(fBegin);
        if (!f.isValid()) {
            break;
        }
        fBegin = f.fEnd;
        f.printFileName();
    }
    File f1(fBegin + 20);
    File f2(f1.fEnd);*/

    /* SUPER FINDER!!!!!!!!!!!! */
    for (int i = 0; i < 10000000; ++i) {
        File f(i);
    }

    return 1;
}