#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#define SECTOR_SIZE 256
#define SECTOR_COUNT 16
#define VTOC_OFFSET SECTOR_COUNT * 17 * SECTOR_SIZE

using namespace std;

class Disk {

	struct VTOC {
		int track;
		int sector;

		init() {
			track = read(VTOC_OFFSET + 1);
		}
	};

	VTOC s;
	fstream disk;

public:

	Disk() {
		disk.open("copy.dsk", ios_base::binary | ios_base::in);
		s.init();
	}

	~Disk() {
		disk.close();
	}

	Disk(const Disk&) = delete;

	Disk& operator=(const Disk&) = delete;

	int read(int offset) {
		disk.seekg(offset);
		return disk.get();
	}

	/*int getNextSector(int this_sector) {
		disk.seekg(this_sector + 1);
		int track = disk.get();
		int sector = disk.get();
		return ((track * SECTOR_COUNT) + sector) * SECTOR_SIZE;
	}

	int getName(int this_sector) {
		disk.seekg(this_sector + 14);
		char name[31];
		disk.get(name, 30);
		string s(name);
		for (int i=0; i<30; i++) cout << name[i];
		cout << s << endl;
		return 0;
	}*/

	void printSector(int offset, int size, int strsize) {
		disk.seekg(offset);
		size /= 2;
		for (int i = 1; i <= size; ++i) {
			cout << hex << setw(2) << setfill('0') << disk.get();
			cout << hex << setw(2) << setfill('0') << disk.get() << " ";
			if (!(i % strsize)) {
				cout << endl;
			}
		}
		cout << endl;
	}

};

int main() {
	Disk disk;
	disk.printSector(VTOC_OFFSET, SECTOR_SIZE, 8);
	return 1;
}