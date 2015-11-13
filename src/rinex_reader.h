#ifndef RINEXREADER_H
#define RINEXREADER_H


// Classes for handling observations RINEX files (data)
#include "Rinex3ObsHeader.hpp"
#include "Rinex3ObsData.hpp"
#include "Rinex3ObsStream.hpp"

// Classes for handling satellite navigation parameters RINEX
// files (ephemerides)
#include "Rinex3NavHeader.hpp"
#include "Rinex3NavData.hpp"
#include "Rinex3NavStream.hpp"

// Classes for handling RINEX files with meteorological parameters
#include "RinexMetBase.hpp"
#include "RinexMetData.hpp"
#include "RinexMetHeader.hpp"
#include "RinexMetStream.hpp"

// Class for handling tropospheric models
#include "TropModel.hpp"

// Class for storing "broadcast-type" ephemerides
#include "GPSEphemerisStore.hpp"

// Class for handling RAIM
#include "PRSolution2.hpp"

// Class defining GPS system constants
#include "GNSSconstants.hpp"

//per convertire coordinate da ecef a lla
#include <Position.hpp>
#include <Triple.hpp>
#include "WGS84Ellipsoid.hpp"

// per calcolare la posizione dei satelliti
#include <Matrix.hpp>
#include <PreciseRange.hpp>

//per leggere il tempo decentemente
#include "CivilTime.hpp"

#include "../../trilateration/src/structs.h"

class RinexReader
{
public:		// Public methods
	RinexReader(char *path_obs, char *path_nav/*, char *path_met = NULL*/);
	int processNextEpoch();
	gpstk::CommonTime getEpochTime();
	static gpstk::CivilTime timePretty(gpstk::CommonTime commonTime);

	Receiver getReceiverEstECEF() const;
	Receiver getReceiverEstLLR() const;

	bool isSolutionValid() const;
	bool isFileFinished() const;

	void printEpochRecap();

	void updateMeasurementAtTime(const gpstk::CommonTime &time);//calcola in un momento a scelta


	std::vector<SatelliteMeasurement> getMeasurements() const;
	std::vector<gpstk::Triple> getSatVelocities() const;

protected:	// Protected methods
	void getEpochMeasures();


protected:	// Protected attributes

	// vediamo se sti 2 van bene qui
	std::vector<gpstk::SatID> prnVec;
	std::vector<double> rangeVec;

	std::vector<SatelliteMeasurement> measurements;
	std::vector<gpstk::Triple> satVelocities;


	bool fileFinished;

	gpstk::GPSEphemerisStore bcestore;	// Object to store ephemeris
	gpstk::PRSolution2 raimSolver;		// RAIM solver
	gpstk::ZeroTropModel noTropModel;	// Object for void-type tropospheric model
	gpstk::TropModel *tropModelPtr;		// Pointer to the tropospheric models.

	const double GAMMA = (gpstk::L1_FREQ_GPS/gpstk::L2_FREQ_GPS)*(gpstk::L1_FREQ_GPS/gpstk::L2_FREQ_GPS);

	int indexP1;
	int indexP2;

	gpstk::Rinex3NavStream rNavFile;    // Object to read Rinex navigation data files
	gpstk::Rinex3NavData rNavData;      // Object to store Rinex navigation data
	gpstk::Rinex3NavHeader rNavHeader;  // Object to read the header of Rinex navigation data files

	gpstk::Rinex3ObsStream rObsFile;    // Object to read Rinex observation data files
	gpstk::Rinex3ObsData rObsData;      // Object to store Rinex observation data
	gpstk::Rinex3ObsHeader rObsHeader;	// Object to read the header of Rinex observation data files

};

#endif // RINEXREADER_H
