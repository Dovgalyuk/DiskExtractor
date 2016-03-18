#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

#define LOGBLOCK_LENGTH 512
#define ALLOCBLOCK_LENGTH LOGBLOCK_LENGTH * 2
#define VINF_OFFSET LOGBLOCK_LENGTH * 2
#define ABMAP_OFFSET VINF_OFFSET + 64
#define FDIR_OFFSET LOGBLOCK_LENGTH * 4

using namespace std;

class Disk {

    fstream disk;
    
    Disk() {
        disk.open("copy.dsk", ios_base::binary | ios_base::in);

        disk.seekg(VINF_OFFSET + 16);
        drBILen    = readByte(2);
        drNmAIBIks = readByte(2);    

        allocBlockMap.resize(drNmAIBIks);
        disk.seekg(ABMAP_OFFSET);
        int temp, val1, val2;
        for (int i = 0; i < drNmAIBIks;) {
            temp = readByte(3);
            val1  = (temp & 0x00FFF000) >> 12;
            val2  = (temp & 0x00000FFF);
            allocBlockMap[i++] = val1;
            allocBlockMap[i++] = val2;
        }
    }

    ~Disk() {
        disk.close();
    }

    Disk(const Disk&) = delete;

    Disk& operator=(const Disk&) = delete;

public:

    int drBILen;
    int drNmAIBIks;
    vector<int> allocBlockMap;

    int readByte(int size) {
        int res = 0;
        for (int i = 0; i < size; ++i) {
            res = res << 8;
            res |= disk.get();
        }
        return res;
    }

    string readString(int size) {
        char *temp = new char[size + 1];
        disk.get(temp, size + 1);
        string stemp(temp);
        delete temp;
        return stemp;
    }

    static fstream& getDiskStream() {
        return getDisk().disk;
    }

    static Disk& getDisk() {
        static Disk _this;
        return _this;
    }

    static int getFContOffset() {
        return FDIR_OFFSET + LOGBLOCK_LENGTH * getDisk().drBILen;
    }

    static int findFile(int offset) {
        Disk::getDiskStream().seekg(offset);
        while (!(Disk::getDisk().readByte(1) && 0x80)) {
            ++offset;
        }
        return offset;
    }

    static void volumeInfo() {
        int temp;

        Disk::getDiskStream().seekg(64 * 16);
        cout << hex;
        cout << "--------------------" << endl;
        cout << "Volume Information" << endl;
        cout << "--------------------" << endl;
        cout << "always $D2D7                          " << Disk::getDisk().readByte(2) << endl;
        cout << "date and time of initialization       " << Disk::getDisk().readByte(4) << endl;
        cout << "date and time of last backup          " << Disk::getDisk().readByte(4) << endl;
        cout << "volume attributes                     " << Disk::getDisk().readByte(2) << endl;
        cout << "number of files in directory          " << Disk::getDisk().readByte(2) << endl;
        cout << "first block of directory              " << Disk::getDisk().readByte(2) << endl;
        cout << "length of directory in blocks         " << Disk::getDisk().readByte(2) << endl;
        cout << "number of allocation blocks on volume " << Disk::getDisk().readByte(2) << endl;
        cout << "size of allocation blocks             " << Disk::getDisk().readByte(4) << endl;
        cout << "number of bytes to allocate           " << Disk::getDisk().readByte(4) << endl;
        cout << "first allocation block in block map   " << Disk::getDisk().readByte(2) << endl;
        cout << "next unused file number               " << Disk::getDisk().readByte(4) << endl;
        cout << "number of unused allocation blocks    " << Disk::getDisk().readByte(2) << endl;
        temp = Disk::getDisk().readByte(1);
        cout << "length of volume name                 " << temp << endl;
        cout << "characters of volume name             " << Disk::getDisk().readString(temp) << endl;
        cout << "--------------------" << endl;

        cout << "Allocation Block Map" << endl;
        cout << "--------------------" << endl;

        int i = 0;
        for (auto &it : getDisk().allocBlockMap) {
            cout << 2 + i++ << "->" << it << " ";
            if (it == 1) {
                cout << endl;
            }
        }
        cout << endl << "--------------------" << endl;
    }
};

/* ------------------------------------------------------------------------- */

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
        Disk::getDiskStream().seekg(offset);
        flFIags  = Disk::getDisk().readByte(1);
        if (flFIags & 0x80) {
            flTyp    = Disk::getDisk().readByte(1);
            flUsrWds = Disk::getDisk().readByte(16);
            flFINum  = Disk::getDisk().readByte(4);
            flStBlk  = Disk::getDisk().readByte(2);
            flLgLen  = Disk::getDisk().readByte(4);
            flPyLen  = Disk::getDisk().readByte(4);
            flRStBlk = Disk::getDisk().readByte(2);
            flRLgLen = Disk::getDisk().readByte(4);
            flRPyLen = Disk::getDisk().readByte(4);
            flCrDat  = Disk::getDisk().readByte(4);
            flMdDat  = Disk::getDisk().readByte(4);
            flNam    = Disk::getDisk().readByte(1);
            flNamS   = Disk::getDisk().readString(flNam);

            fBegin = offset;
            fEnd   = fBegin + 51 + flNam;
            if (fEnd % 2) {
                ++fEnd;
            }
        }
    }

    string printAll() {      
        return string("--------------------") + 
            "\nflags                                   " + to_string(flFIags)  +
            "\nversion number                          " + to_string(flTyp)    +
            "\ninformation used by the Finder          " + to_string(flUsrWds) +
            "\nfile number                             " + to_string(flFINum)  +
            "\nfirst allocation block of data fork     " + to_string(flStBlk)  +
            "\nlogical end-of-file of data fork        " + to_string(flLgLen)  +
            "\nphysical end-of-file of data fork       " + to_string(flPyLen)  +
            "\nfirst allocation block of resource fork " + to_string(flRStBlk) +
            "\nlogical end-of-file of resource fork    " + to_string(flRLgLen) +
            "\nphysical end-of-file of resource fork   " + to_string(flRPyLen) +
            "\ndate and time of creation               " + to_string(flCrDat)  +
            "\ndate and time of last modification      " + to_string(flMdDat)  +
            "\nlength of file name                     " + to_string(flNam)    +
            "\n--------------------\n";
    }

    string printFileName() {
        return string("File name: ") + flNamS + "\n";
    }

    string getValidFileName() {
        string temp = flNamS;
        auto it = temp.find("/");
        while (it < temp.size()) {
            temp.replace(it, 1, "_");
            it = temp.find("/");
        }
        return temp;
    }

    string saveFile() {
        string temp = "save file: \n";

        Disk::getDiskStream().seekg(Disk::getFContOffset() + (flRStBlk - 2) * ALLOCBLOCK_LENGTH);
        for (int i = 0; i < flRLgLen; ++i) {
            temp += (char)Disk::getDiskStream().get();
        }
        return temp;
    }

};

int main() {

    Disk::getDiskStream().seekg(VINF_OFFSET + 12);
    int fileCount = Disk::getDisk().readByte(2);
    vector< File > files;


    int fBegin = FDIR_OFFSET;
    for(int i = 0; i < fileCount; ++i) { 
        fBegin = Disk::findFile(fBegin);
        files.emplace_back(fBegin);
        fBegin = files.back().fEnd;
    }

    Disk::volumeInfo();

    system("rm -r Files");
    system("mkdir Files");
    vector< fstream > fileStreams(fileCount);
    for (int i = 0; i < fileCount; ++i) {
        fileStreams[i].open(string("Files/") + to_string(i) + ":" + files[i].getValidFileName() + ".txt", std::fstream::out);
        if (!fileStreams[i].is_open()) {
            cout << "error: file not created: " << files[i].flNamS + ".txt" << endl;
        }
        fileStreams[i] << files[i].printAll() << endl;
        fileStreams[i] << files[i].saveFile() << endl;
        fileStreams[i].close();
    }




    int q1 = Disk::getFContOffset();
    int q2 = q1 + (files.back().flRStBlk - 2) * ALLOCBLOCK_LENGTH;
    int q3 = q2 + files.back().flRPyLen;

    cout << dec;
    cout << "our first file   " << q1/ 16 << endl;
    cout << "our end file     " << q2 / 16 << endl;
    cout << "our end file end " << q3 / 16 << endl;
    cout << "MB end file      " << (q3 + LOGBLOCK_LENGTH * 3) / 16 << endl;

    Disk::getDiskStream().seekg(0, ios::end);
    cout << "Disk size        " << Disk::getDiskStream().tellg() / 16 << endl;

    return 1;
}