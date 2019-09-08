#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <cmath>
#include <fstream>
#include <cstring>
using namespace std;



int spc_csv(char filename[])
{
	FILE *ptr_myfile;

	// open file
	ptr_myfile = fopen(filename, "rb"); // "rb" to read binary, use "wb" to write binary
	if (!ptr_myfile) {
		printf("Unable to open file!");
		return 1;
	}

	strcpy(filename2 , filename);

	// variables used to store different mem sizes
	int i = 0;				// int, 4 bytes
	unsigned char b = 'a';	// byte, 1 byte
	double d = 0;			// double, 8 bytes
	float f = 0;			// float, 4 bytes
	short int w = 0;		// word, 2 bytes
    
    for(i = 0; filename[i] != '.'; ++i)
	{}
	filename2[++i] = 'c';
	filename2[++i] = 's';
	filename2[++i] = 'v';
    fstream fcsv;
	fcsv.open(filename2, ios_base::out);

	// variables we actually need
	int power2 = 0; // we use this to multiply to the y value (integer data value representing intensity) later
	int numDataPoints = 0; // used to divide first and last x coord, for increments in x values (wavenumbers)
	double firstXCoord = 0; // first logged x value (wavenumber)
	double lastXCoord = 0; // last logged x value (Wavenumber
	int numSubFiles = 1;

	// keep track of data
	// map< subfile#, map< wavenumber, intensity> > data
	map<int, map< double, long double> > data;

	// start main folder
	fread(&b, sizeof(b), 1, ptr_myfile); // flags represent different things
	fread(&b, sizeof(b), 1, ptr_myfile); // spc file version
	fread(&b, sizeof(b), 1, ptr_myfile); // experiment type code
	fread(&b, sizeof(b), 1, ptr_myfile); // IMPORTANT exponenet for Y values
		power2 = (int)b; // save our exponent for multiplying
	fread(&i, sizeof(i), 1, ptr_myfile); // IMPORTANT number of points in file
		numDataPoints = i; // keep this to divide my min and max x values
	fread(&d, sizeof(d), 1, ptr_myfile); // IMPORTANT first x coordinate
		firstXCoord = d; // logs first x value (wavenumber)
	fread(&d, sizeof(d), 1, ptr_myfile); // IMPORTANT last x coordinate
		lastXCoord = d; // logs last x value (wavenumber)
	fread(&i, sizeof(i), 1, ptr_myfile); // IMPORTANT Number of subfiles
		numSubFiles = i; // keep track of how man spectra are being kept track of in this binary file
	fread(&b, sizeof(b), 1, ptr_myfile); // X units type code
	fread(&b, sizeof(b), 1, ptr_myfile); // Y units type code
	fread(&b, sizeof(b), 1, ptr_myfile); // Z units type code
	fread(&b, sizeof(b), 1, ptr_myfile); // Posting disposition
	fread(&i, sizeof(i), 1, ptr_myfile); // compressed date 
	//required
	for (unsigned int j = 0; j < 9; j++) // resolution description text
		fread(&b, sizeof(b), 1, ptr_myfile);
	for (unsigned int j = 0; j < 9; j++) // source instrument description text
		fread(&b, sizeof(b), 1, ptr_myfile);
	fread(&w, sizeof(w), 1, ptr_myfile); // peak point number for interferograms
	for (unsigned int j = 0; j < 8; j++) // spare
		fread(&f, sizeof(f), 1, ptr_myfile);
	for (unsigned int j = 0; j < 130; j++) // Memo
		fread(&b, sizeof(b), 1, ptr_myfile);
	for (unsigned int j = 0; j < 30; j++)  // x, y, and z custom axis strings (combined)
		fread(&b, sizeof(b), 1, ptr_myfile);
	fread(&i, sizeof(i), 1, ptr_myfile); // byte offset to log block
	fread(&i, sizeof(i), 1, ptr_myfile); // file modification flag
	fread(&b, sizeof(b), 1, ptr_myfile); // processing code
	fread(&b, sizeof(b), 1, ptr_myfile); // calibration level + 1
	fread(&w, sizeof(w), 1, ptr_myfile); // sub method sample injection number
	fread(&f, sizeof(f), 1, ptr_myfile); // floatind data multiplier concentration factor
	for (unsigned int j = 0; j < 48; j++)  // method file
		fread(&b, sizeof(b), 1, ptr_myfile);
	fread(&f, sizeof(f), 1, ptr_myfile); // Z subfile increment for even Z multifiles
	fread(&i, sizeof(i), 1, ptr_myfile); // number of w planes
	fread(&f, sizeof(f), 1, ptr_myfile); // w plane increment
	fread(&b, sizeof(b), 1, ptr_myfile); // w axis units code
	for (unsigned int j = 0; j < 187; j++) // reserved
		fread(&b, sizeof(b), 1, ptr_myfile);
	// end main header

	// do this for all subfiles
	for (unsigned int subFile = 0; subFile < numSubFiles; subFile++) {
		// start sub folder for file (Even if only one file here)
		fread(&b, sizeof(b), 1, ptr_myfile); // subfiles flags
		fread(&b, sizeof(b), 1, ptr_myfile); // exponenet for sufiles y values
		if ((int)b != 0) // my files at least had this area blank sinc had only one sub file
			power2 = (int)b; // multiple sub files may have his changed, make sure to check other values for similar things
		fread(&w, sizeof(w), 1, ptr_myfile); // subfile index number
		fread(&f, sizeof(f), 1, ptr_myfile); // subfiels starting z value
		fread(&f, sizeof(f), 1, ptr_myfile); // subfiles ending z value
		fread(&f, sizeof(f), 1, ptr_myfile); // subfiles noise value to use peak picking
		fread(&i, sizeof(i), 1, ptr_myfile); // number of points if XYXY multifile
		fread(&i, sizeof(i), 1, ptr_myfile); // number of co-added scans
		fread(&f, sizeof(f), 1, ptr_myfile); // w axis value
		for (unsigned int j = 0; j < 4; j++) // reserved
			fread(&b, sizeof(b), 1, ptr_myfile);
		// end sub header for file

		// get increment if just lists y values, and not in XY format
		double increment = (lastXCoord - firstXCoord) / (numDataPoints-1);
		double waveNumber = firstXCoord;

		// start data entry for only x values
		for (unsigned int j = 0; j < numDataPoints; j++) {
			fread(&i, sizeof(i), 1, ptr_myfile); // read in data value
			long double intensity = i * pow(2, power2) / (pow(2, 32)); // use pow(2, 16) in bottom fraction instead if data stored as 16-bit rather than 32-bit
			data[subFile][waveNumber] = intensity; // store intensity
			if ( j >= 0)
			fcsv << waveNumber << "," << intensity << endl; // write it to a csv file
			waveNumber += increment; // add increment to x value
		}
		// end data for x values
	}
    fcsv.close();
	fclose(ptr_myfile);
	return 0;
}
