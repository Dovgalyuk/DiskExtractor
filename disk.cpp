#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

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

        Disk::getDiskStream().seekg(512 * 2);
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
        stringstream temp;
        temp << hex;
        temp << "--------------------" << endl;
        temp << "flags                                   " << flFIags  << endl;
        temp << "version number                          " << flTyp    << endl;
        temp << "information used by the Finder          " << flUsrWds << endl;
        temp << "file number                             " << flFINum  << endl;
        temp << "first allocation block of data fork     " << flStBlk  << endl;
        temp << "logical end-of-file of data fork        " << flLgLen  << endl;
        temp << "physical end-of-file of data fork       " << flPyLen  << endl;
        temp << "first allocation block of resource fork " << flRStBlk << endl;
        temp << "logical end-of-file of resource fork    " << flRLgLen << endl;
        temp << "physical end-of-file of resource fork   " << flRPyLen << endl;
        temp << "date and time of creation               " << flCrDat  << endl;
        temp << "date and time of last modification      " << flMdDat  << endl;
        temp << "length of file name                     " << flNam    << endl;
        temp << "--------------------" << endl;
        return temp.str();
    }

    string printFileName() {
        return string("File name: ") + flNamS;
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

    void saveData(fstream& out, int point) {
        int firstOffset = Disk::getFContOffset();
        char str[ALLOCBLOCK_LENGTH + 1];
        while ((point != 1) && (point != 0)) {
            int offset = firstOffset + (point - 2) * ALLOCBLOCK_LENGTH;
            //cout << hex << "block " << point << " offset " << offset << "\n";
            Disk::getDiskStream().seekg(offset);
            Disk::getDiskStream().read(str, ALLOCBLOCK_LENGTH);
            out.write(str, ALLOCBLOCK_LENGTH);    
            //cout.write(str, ALLOCBLOCK_LENGTH);    
            point = Disk::getDisk().allocBlockMap[point - 2];
        }
    }

    void saveFile(string path) {
        string fname;
        if (flRStBlk != 0) {
            fname = path + getValidFileName() + ".resourсe";
            //cout << fname << "\n";
            fstream file(fname, ios_base::binary | ios_base::out);
            if (!file.is_open()) {
                cout << "error: file not created: " << flNamS << endl;
            }
            saveData(file, flRStBlk);
            file.close();
        }

        if (flStBlk != 0) {
            fname = path + getValidFileName() + ".data";
            //cout << fname << "\n";
            fstream file(fname, ios_base::binary | ios_base::out);
            if (!file.is_open()) {
                cout << "error: file not created: " << flNamS << endl;
            }
            saveData(file, flStBlk);
            file.close();
        }
    }

};

void generInfoFile(string path, vector< File > files) {
    fstream file(path + "Files.info", ios_base::binary | ios_base::out);
    if (!file.is_open()) {
        cout << "error: info-file not created: " << endl;
    }

    file << hex;
    for (int i = 0; i < files.size(); ++i) {
        file << files[i].printFileName() << endl;
        file << files[i].printAll() << endl;
        file << "firstOffset resource fork:  " << (!files[i].flRStBlk ? 0 : Disk::getFContOffset() + (files[i].flRStBlk - 2) * ALLOCBLOCK_LENGTH) << endl;
        file << "firstOffset data fork:      " << (!files[i].flStBlk  ? 0 : Disk::getFContOffset() + (files[i].flStBlk  - 2) * ALLOCBLOCK_LENGTH) << endl;
        file << endl << endl;
    }

    file.close();
}

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
    for (int i = 0; i < fileCount; ++i) {
        files[i].saveFile("Files/");
    }

    generInfoFile("Files/", files);

    return 1;
}
