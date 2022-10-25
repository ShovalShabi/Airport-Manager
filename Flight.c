#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Flight.h"
#include "fileHelper.h"

//Presenter:Shoval Shabi

void	initFlight(Flight* pFlight, const AirportManager* pManager)
{
	Airport* pPortOr = setAiportToFlight(pManager, "Enter name of origin airport:");
	pFlight->nameSource = _strdup(pPortOr->name);
	int same;
	Airport* pPortDes;
	do {
		pPortDes = setAiportToFlight(pManager, "Enter name of destination airport:");
		same = isSameAirport(pPortOr, pPortDes);
		if (same)
			printf("Same origin and destination airport\n");
	} while (same);
	pFlight->nameDest = _strdup(pPortDes->name);
	initPlane(&pFlight->thePlane);
	getCorrectDate(&pFlight->date);
}

int		isFlightFromSourceName(const Flight* pFlight, const char* nameSource)
{
	if (strcmp(pFlight->nameSource, nameSource) == 0)
		return 1;
		
	return 0;
}


int		isFlightToDestName(const Flight* pFlight, const char* nameDest)
{
	if (strcmp(pFlight->nameDest, nameDest) == 0)
		return 1;

	return 0;


}

int		isPlaneCodeInFlight(const Flight* pFlight, const char*  code)
{
	if (strcmp(pFlight->thePlane.code, code) == 0)
		return 1;
	return 0;
}

int		isPlaneTypeInFlight(const Flight* pFlight, ePlaneType type)
{
	if (pFlight->thePlane.type == type)
		return 1;
	return 0;
}


void	printFlight(const Flight* pFlight)
{
	printf("Flight From %s To %s\t",pFlight->nameSource, pFlight->nameDest);
	printDate(&pFlight->date);
	printPlane(&pFlight->thePlane);
}

void	printFlightV(const void* val)
{
	const Flight* pFlight = *(const Flight**)val;
	printFlight(pFlight);
}


Airport* setAiportToFlight(const AirportManager* pManager, const char* msg)
{
	char name[MAX_STR_LEN];
	Airport* port;
	do
	{
		printf("%s\t", msg);
		myGets(name, MAX_STR_LEN,stdin);
		port = findAirportByName(pManager, name);
		if (port == NULL)
			printf("No airport with this name - try again\n");
	} while(port == NULL);

	return port;
}

void	freeFlight(Flight* pFlight)
{
free(pFlight->nameSource);
free(pFlight->nameDest);
free(pFlight);
}


int saveFlightToFile(const Flight* pF, FILE* fp)
{
	if (!writeStringToFile(pF->nameSource, fp, "Error write flight source name\n"))
		return 0;

	if (!writeStringToFile(pF->nameDest, fp, "Error write flight destination name\n"))
		return 0;

	if (!savePlaneToFile(&pF->thePlane, fp))
		return 0;

	if (!saveDateToFile(&pF->date, fp))
		return 0;

	return 1;
}


int loadFlightFromFile(Flight* pF, const AirportManager* pManager, FILE* fp)
{

	pF->nameSource = readStringFromFile(fp, "Error reading source name\n");
	if (!pF->nameSource)
		return 0;

	if (findAirportByName(pManager, pF->nameSource) == NULL)
	{
		printf("Airport %s not in manager\n", pF->nameSource);
		free(pF->nameSource);
		return 0;
	}

	pF->nameDest = readStringFromFile(fp, "Error reading destination name\n");
	if (!pF->nameDest)
	{
		free(pF->nameSource);
		return 0;
	}

	if (findAirportByName(pManager, pF->nameDest) == NULL)
	{
		printf("Airport %s not in manager\n", pF->nameDest);
		free(pF->nameSource);
		free(pF->nameDest);
		return 0;
	}

	if (!loadPlaneFromFile(&pF->thePlane, fp))
	{
		free(pF->nameSource);
		free(pF->nameDest);
		return 0;
	}


	if (!loadDateFromFile(&pF->date, fp))
	{
		free(pF->nameSource);
		free(pF->nameDest);
		return 0;
	}

	return 1;
}

int loadFlightFromCompressFile(Flight* pFlight, FILE* pFile)
{
	BYTE recievedData[6];
	if (fread(&recievedData, sizeof(BYTE), 6, pFile) != 6)return 0;
	int srcNameLength = (recievedData[0] >> 3) & 0x1F; //getting the src length name
	int destNameLength = ((recievedData[0]<<2) & 0x1F) | ((recievedData[1] >> 6) & 0x3); //getting the dest length name
	int planeType = (recievedData[1] >> 4) & 0x3;//getting plane type
	int month = recievedData[1] & 0xF;//getting month

	int letterCode0 = (recievedData[2] >> 3) & 0x1F;//CODE[0] in ASCII code
	pFlight->thePlane.code[0] =(char) (letterCode0+65);//conversion back to char

	int letterCode1 = ((recievedData[2] & 0x7)<<2) | ((recievedData[3] >> 6) & 0x3);//CODE[1] in ASCII code
	pFlight->thePlane.code[1] = (char)(letterCode1 + 65) ;//conversion back to char

	int letterCode2 = (recievedData[3] >> 1) & 0x1F;//CODE[2] in ASCII code
	pFlight->thePlane.code[2] = (char)(letterCode2 + 65);//conversion back to char

	int letterCode3 = ((recievedData[3] & 0x1)<<4) | ((recievedData[4] >> 4) & 0xF);//CODE[3] in ASCII code
	pFlight->thePlane.code[3] = (char)(letterCode3 + 65);//conversion back to char

	pFlight->thePlane.code[4] = 0;
	pFlight->thePlane.type = planeType;
	int year = recievedData[4] & 0xF;//getting year
	int day = recievedData[5] & 0x1F;//getting days 
	pFlight->nameSource = (char*)calloc(srcNameLength + 1, sizeof(char));//calloc and plus 1 for the '\0' 
	if (!pFlight->nameSource)return 0;
	if (fread(pFlight->nameSource, sizeof(char), srcNameLength, pFile) != srcNameLength)//reading source name
	{
		free(pFlight->nameSource);
		return 0;
	}
	pFlight->nameDest = (char*)calloc(destNameLength + 1, sizeof(char));//calloc and plus 1 for the '\0' 
	if (!pFlight->nameDest)
	{
		free(pFlight->nameSource);
		return 0;
	}
	if (fread(pFlight->nameDest, sizeof(char), destNameLength, pFile) != destNameLength)//reading dest name
	{
		free(pFlight->nameSource);
		free(pFlight->nameDest);
		return 0;
	}
	//putting values within date in flight
	pFlight->date.year = year+2021;
	pFlight->date.month = month;
	pFlight->date.day = day;

	return 1;
}

int saveFlightToCompressFile(Flight* pFlight, FILE* pFile)
{
	BYTE insertData[6];
	insertData[0] = (BYTE)(((int)strlen(pFlight->nameSource) << 3) | ((int)strlen(pFlight->nameDest) >> 2));//setting the first byte
	insertData[1] = (BYTE)((int)strlen(pFlight->nameDest) << 6) | (pFlight->thePlane.type << 4) | (pFlight->date.month);//setting the second byte
	int code0 = (int)pFlight->thePlane.code[0] - 65;
	int code1 = (int)pFlight->thePlane.code[1] - 65;
	int code2 = (int)pFlight->thePlane.code[2] - 65;
	int code3 = (int)pFlight->thePlane.code[3] - 65;
	insertData[2] = (BYTE)((code0 << 3) | (code1 >> 2));
	insertData[3] = (BYTE)((code1 << 6) | (code2 << 1) | (code3>>4));
	insertData[4] = (BYTE)((code3 << 4) | (pFlight->date.year-2021));
	insertData[5] = (BYTE)((pFlight->date.day));
	if (fwrite(&insertData, sizeof(BYTE), 6, pFile) != 6)return 0;
	if (fwrite(pFlight->nameSource, sizeof(char), strlen(pFlight->nameSource), pFile) != strlen(pFlight->nameSource))return 0;
	if (fwrite(pFlight->nameDest, sizeof(char), strlen(pFlight->nameDest), pFile) != strlen(pFlight->nameDest))return 0;
	return 1;
}

int	compareFlightBySourceName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameSource, pFlight2->nameSource);
}

int	compareFlightByDestName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameDest, pFlight2->nameDest);
}

int	compareFlightByPlaneCode(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->thePlane.code, pFlight2->thePlane.code);
}

int		compareFlightByDate(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;


	return compareDate(&pFlight1->date, &pFlight2->date);
	

	return 0;
}

