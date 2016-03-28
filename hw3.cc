/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Name                     :Isaac Styles
// Department Name : Computer and Information Sciences
// File Name                :hw3.cc
// Purpose                  :Read file. Create five 'Stations' which pipe Product_Records sequentially
//                                      until arriving back to parent. Then write results to file.
// Author			        : Isaac Styles, styles@goldmail.etsu.edu
// Create Date	            :Feb 26, 2016
//
//-----------------------------------------------------------------------------------------------------------
//
// Modified Date	: Feb 29, 2016
// Modified By		: Isaac Styles
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define DEBUG
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "product_record.h"

using namespace std;
//Function prototypes
void station0();
void station1();
void station2();
void station3();
void station4();
void parentFunction(char**, int[], int*);
int readRecordFromFile(fstream&, product_record&);
int writeRecordToFile(fstream&, product_record& p);
void displayRecord(product_record&);

//Global data
int pipes [6][2];                                   //pipes for communication between children and parent


int main(int argc, char** argv)
{
    int children[5];                                //array of child PIDs

    int errorCode = 0;                              //exit code
    if(argc == 3)                                   //if proper number of command arguments
    {
        int pipeErr = 0;                            //holds error code for pipe creation

        for (size_t t = 0; t<=6 ; t++)              //init pipes for children
        {
            pipeErr = pipe(pipes[t]);
            if (pipeErr < 0)                        //on pipe init error display message and set errorCode
            {
                cout << "FATAL ERROR:The pipe could not be created." << endl;
                errorCode--;                        //exit code < 0
            }
        }
        for(int child = 0; child<=4; child++)       //fork children
        {
            children[child] = fork();               //children = PID of child process

            if (children[child] == -1)              //if failure to create child
            {
                cout << "Could not create child process." << endl;
                exit(-1);
            }
            else
            {
                if (children[child] == 0)           //child functions
                {
                    if(child == 0){
                        station0();
                    }
                    else if(child == 1){
                        station1();
                    }
                    else if(child == 2){
                        station2();
                    }
                    else if(child == 3){
                        station3();
                    }
                    else if(child == 4){
                        station4();
                    }
                }
                else                                //parent function
                {
                    if(child == 4){                 //after Stations are initialized, run parent code
                        parentFunction(argv, children, &errorCode);
                    }
                }
            }
        }
    }
    else{
        cout << "Please use command arguments for infile and outfile." << endl;
    }
    return errorCode;
}
/// <summary>
/// Compute tax for a product_record, then depending on idnumber, pipe the record to either station 1 or 2
/// </summary>
void station0()
{
    //cout << " ST0 My PID is " << getpid() << endl << " My Parent's PID is " << getppid() << endl;
    size_t processed = 0;                                           //number of records processed
    product_record pr;
    read(pipes[0][0], (char*)&pr, sizeof(product_record));          //initial blocking read
    do
    {
        if (pr.idnumber == -1)                                      //for handling of empty file
        {
            break;
        }
        processed++;                                                //increment number records processed
        pr.tax = pr.price*pr.number * 0.05;                         //calculuate tax
        pr.stations[0] = 1;

        if (pr.idnumber >= 1000)                                    //if idnumber indicates free shipping
        {
            write(pipes[2][1],(char*)&pr, sizeof(product_record));  //write sANDh free to station 2
        }
        else
        {
            write(pipes[1][1],(char*)&pr, sizeof(product_record));  //write to sANDh - station 1
        }
        read(pipes[0][0], (char*)&pr, sizeof(product_record));      //get another record
    }while(pr.idnumber != -1);
    cout << "Station 0 processed " << processed <<" product records."<< endl;
    write(pipes[1][1],(char*)&pr, sizeof(product_record));          //write terminating record to station 1
    exit(0);
}
/// <summary>
/// compute shipping and handling
/// </summary>
void station1()
{
    //cout << " ST1 My PID is " << getpid() << endl << " My Parent's PID is " << getppid() << endl;
    size_t processed = 0;                                           //number of records processed
    product_record pr;
    read(pipes[1][0], (char*)&pr, sizeof(product_record));          //initial blocking read
    do
    {
        if (pr.idnumber == -1)                                      //for handling of empty file
        {
            break;
        }
        processed++;                                                //increment number records processed
        pr.sANDh = (pr.price*pr.number)*.01 + 10;
        pr.stations[1] = 1;
        write(pipes[2][1],(char*)&pr, sizeof(product_record));
        read(pipes[1][0], (char*)&pr, sizeof(product_record));      //get another record
    }while(pr.idnumber != -1);
    cout << "Station 1 processed " << processed <<" product records."<< endl;
    write(pipes[2][1],(char*)&pr, sizeof(product_record));          //write terminating record to station 2
    exit(0);
}
/// <summary>
/// compute order total
/// </summary>
void station2()
{
    //cout << " ST2 My PID is " << getpid() << endl << " My Parent's PID is " << getppid() << endl;
    size_t processed = 0;                                               //number of records processed
    product_record pr;
    read(pipes[2][0], (char*)&pr, sizeof(product_record));          //initial blocking read
    do
    {
        if (pr.idnumber == -1)                                      //for handling of empty file
        {
            break;
        }
        processed++;                                                //increment number records processed
        pr.total = (pr.number*pr.price) + pr.sANDh + pr.tax;
        pr.stations[2] = 1;

        write(pipes[3][1],(char*)&pr, sizeof(product_record));
        read(pipes[2][0], (char*)&pr, sizeof(product_record));      //get another record
    }while(pr.idnumber != -1);
    cout << "Station 2 processed " << processed <<" product records."<< endl;
    write(pipes[3][1],(char*)&pr, sizeof(product_record));          //write terminating record to station 3
    exit(0);
}
/// <summary>
/// compute and display running total
/// </summary>
void station3()
{
    //cout << " ST3 My PID is " << getpid() << endl << " My Parent's PID is " << getppid() << endl;
    size_t processed = 0;                                               //number of records processed
    product_record pr;
    double runningTotal = 0;                                        //holds the total of records processed
    read(pipes[3][0], (char*)&pr, sizeof(product_record));          //initial blocking read
    do
    {
        if (pr.idnumber == -1)                                      //for handling of empty file
        {
            break;
        }
        processed++;                                                //increment number records processed
        runningTotal += pr.total;
        pr.stations[3] = 1;
        cout << "Running Total: " <<runningTotal<< endl;
        write(pipes[4][1],(char*)&pr, sizeof(product_record));      //write current record to station 4
        read(pipes[3][0], (char*)&pr, sizeof(product_record));      //get another record
    }while(pr.idnumber != -1);
    cout << "Station 3 processed " << processed <<" product records."<< endl;
    write(pipes[4][1],(char*)&pr, sizeof(product_record));          //write terminating record to station 4
    exit(0);
}
/// <summary>
/// display the record and pipe back to parent
/// </summary>
void station4()
{
    //cout << " ST4 My PID is " << getpid() << endl << " My Parent's PID is " << getppid() << endl;
    size_t processed = 0;                                               //number of records processed
    product_record pr;
    read(pipes[4][0], (char*)&pr, sizeof(product_record));          //initial blocking read
    do
    {
        if (pr.idnumber == -1)                                      //for handling of empty file
        {
            break;
        }
        processed++;                                                //increment number records processed
        pr.stations[4] = 1;
        displayRecord(pr);
        write(pipes[5][1],(char*)&pr, sizeof(product_record));      //write back to parent
        read(pipes[4][0], (char*)&pr, sizeof(product_record));      //get another record
    }while(pr.idnumber != -1);
    cout << "Station 4 processed " << processed <<" product records."<< endl;
    exit(0);
}
/// <summary>
/// Writes PRODUCT_RECORDS to Station 1 pipe, Reads PRODUCT_RECORDS from Station 4 pipe,
///     then sends the terminate record.
/// </summary>
void parentFunction(char** argv, int children [], int* errorCode)
{
    size_t recordsIn = 0;                           //number of records read
    size_t recordsRcvd = 0;                         //number of records received
    int readError = 0;                              //error code for write process
    product_record pr;                              //holder for current record
    fstream fin (argv[1],fstream::in);			    //input file
    readError = readRecordFromFile(fin, pr);        //get a record from file
    while (readError == 0)                          //while more records in file
    {
        recordsIn++;
        write(pipes[0][1],(char*)&pr, sizeof(product_record));  //write record to station 1 pipe
        readError = readRecordFromFile(fin, pr);    //get another record
    }
    if (readError < -1)                             //if an actual read error occurred, add to exit code
    {
        errorCode += readError;                     //read in PRODUCT_RECORDS
    }
    fin.close();                                    //close infile filestream



    fstream fout (argv[2],fstream::out);			// initialize output file
    while(recordsRcvd < recordsIn)                 //while there are unprocessed records
    {
        recordsRcvd++;
        read(pipes[5][0], (char*)&pr, sizeof(product_record));      //get record from children
        *(errorCode) += writeRecordToFile(fout, pr);                //write record to file
    }
    fout.close();                                   //close output filestream
    pr = product_record();                          //initialize terminate record
    pr.idnumber= -1;
    write(pipes[0][1],(char*)&pr, sizeof(product_record));  //write terminate record to station 1 pipe
    cout << "The parent processed " << recordsRcvd <<" product orders."<< endl;
    wait(&children[0]);
    wait(&children[1]);
    wait(&children[2]);
    wait(&children[3]);
    wait(&children[4]);
}
/// <summary>
/// Displays an array of product_record to standardoutput
/// </summary>
/// <param name="p">The array of product_record</param>
void displayRecord(product_record& p)
{
    cout << "idnumber = " << p.idnumber << endl;
    cout << "    name = " << p.name << endl;
    cout << "   price = " << p.price << endl;
    cout << "  number = " << p.number << endl;
    cout << "     tax = " << p.tax << endl;
    cout << "     s&h = " << p.sANDh << endl;
    cout << "   total = " << p.total << endl;
    cout << "stations = " << p.stations[0] << " " << p.stations[1] << " " << p.stations[2] << " " << p.stations[3] << " " << p.stations[4] << " " << endl;
    cout << "\n\tEND RECORD" << endl;
}
/// <summary>
/// Creates an array of product_records from a file.
/// </summary>
/// <param name="f">The file stream.</param>
/// <param name="pp">The ref to product_record</param>
/// <param name="objs">The number objs read in.</param>
/// <returns>-1 if failure, 0 if success</returns>
int readRecordFromFile(fstream& f, product_record& p)
{
	//int objs = 0;											//number of objects read in = 0
	string line;										    //current input line

	if (!f)
	{
		cout << "could not read input file" << endl;
		return -2;
	}
	else
	{
        int errorcode = -1;                                 //return -1 if empty file
		if (getline(f, line))
		{
		    p.idnumber = atoi(line.c_str());			//idnumber
			getline(f, line);							//name
			strncpy(p.name,line.c_str(), line.length()+1);			//copy name to struct
			getline(f,line);							//price
			p.price = atof(line.c_str());
			getline(f, line);							//number
			p.number = atoi(line.c_str());
			getline(f, line);							//tax
			p.tax = atof(line.c_str());
			getline(f, line);							//sANDh
			p.sANDh = atof(line.c_str());
			getline(f, line);							//total
			p.total = atof(line.c_str());
			getline(f, line);							//read stations vector
			p.stations[0] = 0;
			p.stations[1] = 0;
			p.stations[2] = 0;
			p.stations[3] = 0;
			p.stations[4] = 0;
			errorcode = 0;
		}
		return errorcode;
	}
}
/// <summary>
/// Writes product_record to opened file.
/// </summary>
/// <param name="f">The file stream</param>
/// <param name="p">The pointer to product_record</param>
/// <returns>-1 on failure, 0 on success</returns>
int writeRecordToFile(fstream& f, product_record& p)
{
	// For practice, open an output file
	// Options:
	//    fstream::out             for write, kills existing file
	//    fstream::in              for read-only
	//    fstream::in | ios::out   for read/write
	//    fstream::binary          for binary access

	// Always check that the file opened correctly
	if (!f)
	{
		// f.open( ) failed
		cout << "output file failed on open" << endl;
		return -1;
	}
	else {
        f << p.idnumber << endl;
        f << p.name << endl;
        f << p.price << endl;
        f << p.number << endl;
        f << p.tax << endl;
        f << p.sANDh << endl;
        f << p.total << endl;
        f << p.stations[0] << " " << p.stations[1] << " " << p.stations[2] << " " << p.stations[3] << " " << p.stations[4] << " " << endl;
    }
	return 0;
}
