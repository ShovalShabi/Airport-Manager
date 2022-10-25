#pragma once
#include "AirportManager.h"
#include "Airline.h"

//Presenter:Shoval Shabi

typedef enum
{
	eAddFlight, eAddAirport, ePrintCompany, ePrintAirports,eSortFlights, eSearchFlight,
	ePrintNumFlightsOrig, ePrintFlightsPlaneCode, ePrintFlightsPlaneType, eNofOptions
} eMenuOptions;

const char* str[eNofOptions];

int initManagerAndAirline(AirportManager* pManager, int* compress,Airline* pCompany,const char* airlineFileName);
int menu();


#define EXIT -1
#define MANAGER_FILE_NAME "airport_authority.txt"
#define AIRLINE_FILE_NAME "airline.bin"
#define COMPRESS
# define AIRLINE_COMPRESS_FILE_NAME "airline_compress.bin"


