#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
	int not_gate = 0;
	int and_gate = 0;
	int not_count = stoi(argv[1]);
	int and_count = stoi(argv[1]);

	fstream fi("genTest.blif", ios::out);
	fi << ".model genTest" << endl;
	fi << ".inputs I1 I2 " << flush;
	for(int i=0; i<and_count; i++) {
		fi << "i" << i << " ";
	}
	fi << "\n.outputs " << flush;
	for(int i=0; i<not_count; i++) {
		fi << "n" << i << " ";
	}
	fi << "a" << and_count << endl;
	fi << ".names I1 I2 A1\n"
	   << "11 1\n"
	   << ".names I2 A1 A2\n"
	   << "11 1" << endl;
	for(int i=0; i<not_count; ++i) {
		fi << ".names A2 n" << i
		   << "\n0 1" << endl;
	}
	fi << ".names I1 I2 a0"
	   << "\n11 1" << endl;
	for(int i=1; i<and_count+1; ++i) {
		fi << ".names a" << i-1 << " i" << i-1 << " a" << i
		   << "\n11 1" << endl;
	}
	fi << ".end" << endl;
}