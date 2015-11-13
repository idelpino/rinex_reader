#include "rinex_reader.h"

using namespace gpstk;
using namespace std;

RinexReader::RinexReader(char *path_obs, char *path_nav) :
	rNavFile(path_nav),		//reads the nav file
	rObsFile(path_obs)		//reads the obs file
{
	// No other epochs to read
	fileFinished = false;

	// se non ho il file meteo, assegno il modello troposferico void
	tropModelPtr = &noTropModel;

	/// Parse nav file
	rNavFile >> rNavHeader;
	while(rNavFile >> rNavData) bcestore.addEphemeris(rNavData);

	// Setting the criteria for looking up ephemeris
	bcestore.SearchNear();

	rObsFile >> rObsHeader;		// Let's read the obs' header

	// The following lines fetch the corresponding indexes for some observation types
	// we are interested in. Given that old-style observation types are used, GPS is assumed.
	try
	{
		indexP1 = rObsHeader.getObsIndex( "P1" );
	}
	catch(...)
	{
		cerr << "The observation file doesn't have P1 pseudoranges." << endl;
		exit(1);
	}

	try
	{
		indexP2 = rObsHeader.getObsIndex( "P2" );
	}
	catch(...)
	{
		indexP2 = -1;
	}

	// The default constructor for PRSolution2 objects (like "raimSolver")
	// is to set a RMSLimit of 6.5. We change that here.
	// With this value of 3e6 the solution will have a lot more dispersion.
	raimSolver.RMSLimit = 3e6;

}

int RinexReader::processNextEpoch()
{
	if(rObsFile >> rObsData)
	{
		fileFinished = false;

		if(rObsData.epochFlag == 0 || rObsData.epochFlag == 1 )
		{
//			//TODO da mettere nella classe probabilmente
//			vector<SatID> prnVec;
//			vector<double> rangeVec;
			prnVec.clear();
			rangeVec.clear();

			getEpochMeasures(); //salva in prnVec e rangeVec

			measurements = computeSatPosition(rObsData.time);



			int ret;
			// Compute positon
			ret = raimSolver.RAIMCompute( rObsData.time,		// current time
									prnVec,			// vector of visible satellites
									rangeVec,		// vector of corresponding ranges
									bcestore,		// satellite ephemerides
									tropModelPtr );	// pointer to the tropospheric model to be applied

			if (ret!=0)
				cout << "Return value of raimSolver.RAIMCompute:" << ret << endl;

			//TODO guarda se ho assegnato la soluzione ai cosi della classe


		}
	} else {
		fileFinished = true;
	}

	return 0;//TODO torna i codici sensati di raimcompute ad esempio
}

CommonTime RinexReader::getEpochTime()
{
	return rObsData.time;
}

CivilTime RinexReader::timePretty(CommonTime commonTime)
{
	return CivilTime(commonTime);
}

Receiver RinexReader::getReceiverEstECEF() const
{
	Receiver r;

	if( raimSolver.isValid() )
	{
		r.pos = Point<double>(raimSolver.Solution[0], raimSolver.Solution[1], raimSolver.Solution[2]);
		r.bias = raimSolver.Solution[3];
	}

	return r;
}

Receiver RinexReader::getReceiverEstLLR() const
{
	Receiver r;

	if( raimSolver.isValid() )
	{
		Triple sol_ecef, sol_llr;
		sol_ecef[0] = raimSolver.Solution[0];
		sol_ecef[1] = raimSolver.Solution[1];
		sol_ecef[2] = raimSolver.Solution[2];

		// conversione in coordinate geodetic
		WGS84Ellipsoid WGS84;
		double AEarth = WGS84.a();
		double eccSquared = WGS84.eccSquared();

		Position::convertCartesianToGeodetic(sol_ecef, sol_llr, AEarth , eccSquared);

		r.pos = Point<double>(sol_llr[0], sol_llr[1], sol_llr[2]);
		r.bias = raimSolver.Solution[3];
	}

	return r;
}

bool RinexReader::isSolutionValid() const
{
	return raimSolver.isValid();
}

bool RinexReader::isFileFinished() const
{
	return fileFinished;
}

void RinexReader::printEpochRecap()
{
	cout << "### Epoch: " << timePretty(getEpochTime()) << endl;
	cout << setprecision(12);

	// If we got a valid solution, let's print it
	if( raimSolver.isValid() )
	{
		cout << "### ECEF: " << getReceiverEstECEF().toString() << endl;
		cout << "### LLR: " << getReceiverEstLLR().toString() << endl ;

	} else {
		cout << "Solution is not valid\n";
	}
}

// This part gets the PRN numbers and ionosphere-corrected
// pseudoranges for the current epoch. They are correspondly fed
// into "prnVec" and "rangeVec";
void RinexReader::getEpochMeasures()
{

	Rinex3ObsData::DataMap::const_iterator it;

	for( it = rObsData.obs.begin(); it!= rObsData.obs.end(); it++ )
	{
		// The RINEX file may have P1 observations, but the current
		// satellite may not have them.
		double P1( 0.0 );
		try
		{
			P1 = rObsData.getObs( (*it).first, indexP1 ).data;
		}
		catch(...)
		{
			// Ignore this satellite if P1 is not found
			continue;
		}

		double ionocorr( 0.0 );

		// If there are P2 observations, let's try to apply the
		// ionospheric corrections
		if( indexP2 >= 0 )
		{

			// The RINEX file may have P2 observations, but the
			// current satellite may not have them.
			double P2( 0.0 );
			try
			{
				P2 = rObsData.getObs( (*it).first, indexP2 ).data;
			}
			catch(...)
			{
				// Ignore this satellite if P1 is not found
				continue;
			}

			// Vector 'vecData' contains RinexDatum, whose public
			// attribute "data" indeed holds the actual data point
			ionocorr = 1.0 / (1.0 - GAMMA) * ( P1 - P2 );

		}

		// Now, we include the current PRN number in the first part
		// of "it" iterator into the vector holding the satellites.
		// All satellites in view at this epoch that have P1 or P1+P2
		// observations will be included.
		prnVec.push_back( (*it).first );

		// The same is done for the vector of doubles holding the
		// corrected ranges
		rangeVec.push_back( P1 - ionocorr );

		// WARNING: Please note that so far no further correction
		// is done on data: Relativistic effects, tropospheric
		// correction, instrumental delays, etc.

	}  // End of 'for( it = rObsData.obs.begin(); it!= rObsData.obs.end(); ...'
}

std::vector<SatelliteMeasurement> RinexReader::getMeasurements() const
{
	return measurements;
}

vector<SatelliteMeasurement> RinexReader::computeSatPosition(const CommonTime &time)
{
	int ret;
	Matrix<double> calcPos;
	vector<SatelliteMeasurement> satPos;

	ret = raimSolver.PrepareAutonomousSolution(time, //questo e' il TOA (time of arrival), cioe' l'istante nel quale voglio predire la posizione dei satelliti
										 prnVec,
										 rangeVec,
										 bcestore,
										 calcPos); //satellite positions at transmit time, and the corrected pseudorange
	//Return values: 0 ok -4 ephemeris not found for all the satellites
	//NB: verify that the number of good entries (Satellite[.] > 0) is > 4 before proceeding

	if (ret!=0)
		cout << "Return value of raimSolver2.PrepareAutonomousSolution:" << ret << endl;


	for (size_t i = 0; i < prnVec.size(); ++i)
	{
		SatelliteMeasurement sm;
		sm.pos = Point<double>(calcPos[i][0], calcPos[i][1], calcPos[i][2]);
		sm.pseudorange = calcPos[i][3];

		satPos.push_back(sm);
	}

	return satPos;
}
