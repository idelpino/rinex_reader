#include <iostream>

#include "rinex_reader.h"

using namespace std;

int main(int argc, char** argv)
{
	if( (argc < 2) || (argc > 3) )
	{
		cerr <<  "Usage:\n\t" << argv[0]
			 << " <RINEX Obs file>  <RINEX Nav file>" /* [<RINEX Met file>]"*/ << endl;

		exit (-1);
	}


	RinexReader rr(argv[1], argv[2]);


	while (!rr.isFileFinished()) {

		rr.processNextEpoch();
		rr.printEpochRecap();

		cout << "Do you want to see the evolution of a satellite in that epoch? y/n\n";
		char a;
		cin >> a;

		if (a == 'y')
		{
			for (int i = 0; i < 30; ++i) {
				cout << rr.computeSatPosition(rr.getEpochTime() + i)[0].toString() << endl;

			}
			cout << endl;
		}
	}



    return 0;
}

