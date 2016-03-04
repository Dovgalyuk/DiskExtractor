#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

#define FDIR_OFFSET 128 * 16

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

    static int findFile(int offset) {
        Disk::getDisk().seekg(offset);
        while (!(Disk::read(1) && 0x80)) {
            ++offset;
        }
        return offset;
    }

    static void volumeInfo() {
        Disk::getDisk().seekg(64 * 16);
        cout << hex;
        cout << "--------------------" << endl;
        cout << "always $D2D7                          " << Disk::read(2) << endl;
        cout << "date and time of initialization       " << Disk::read(4) << endl;
        cout << "date and time of last backup          " << Disk::read(4) << endl;
        cout << "volume attributes                     " << Disk::read(2) << endl;
        cout << "number of files in directory          " << Disk::read(2) << endl;
        cout << "first block of directory              " << Disk::read(2) << endl;
        cout << "length of directory in blocks         " << Disk::read(2) << endl;
        cout << "number of allocation blocks on volume " << Disk::read(2) << endl;
        cout << "size of allocation blocks             " << Disk::read(4) << endl;
        cout << "number of bytes to allocate           " << Disk::read(4) << endl;
        cout << "first allocation block in block map   " << Disk::read(2) << endl;
        cout << "next unused file number               " << Disk::read(4) << endl;
        cout << "number of unused allocation blocks    " << Disk::read(2) << endl;
        cout << "length of volume name                 " << Disk::read(1) << endl;
        cout << "characters of volume namealways $D2D7 " << Disk::read(4) << endl;
        cout << "--------------------" << endl;
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
        if (flFIags & 0x80) {
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
        }
    }

    void printAll() {
        cout << hex;
        cout << "--------------------" << endl;
        cout << "flags                                   " << flFIags  << endl;
        cout << "version number                          " << flTyp    << endl;
        cout << "information used by the Finder          " << flUsrWds << endl;
        cout << "file number                             " << flFINum  << endl;
        cout << "first allocation block of data fork     " << flStBlk  << endl;
        cout << "logical end-of-file of data fork        " << flLgLen  << endl;
        cout << "physical end-of-file of data fork       " << flPyLen  << endl;
        cout << "first allocation block of resource fork " << flRStBlk << endl;
        cout << "logical end-of-file of resource fork    " << flRLgLen << endl;
        cout << "physical end-of-file of resource fork   " << flRPyLen << endl;
        cout << "date and time of creation               " << flCrDat  << endl;
        cout << "date and time of last modification      " << flMdDat  << endl;
        cout << "length of file name                     " << flNam    << endl;
        cout << "--------------------" << endl;
    }

    void printFileName() {
        cout << "File name: " << flNamS << endl;
    }

};

int main() {

    Disk::getDisk().seekg(64 * 16 + 12);
    int fileCount = Disk::read(2);
    vector< File > files;


    int fBegin = FDIR_OFFSET;
    for(int i = 0; i < fileCount; ++i) { 
        fBegin = Disk::findFile(fBegin);
        files.emplace_back(fBegin);
        fBegin = files.back().fEnd;

        cout << "counter: " << i << endl;
        files.back().printFileName();
        files.back().printAll();
        cout << endl << endl;
    }

    Disk::volumeInfo();

    return 1;
}