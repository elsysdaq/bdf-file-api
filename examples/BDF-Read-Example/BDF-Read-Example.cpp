// ********************************************************************************/
/* BDF API Example
/*
/* This short example shows how to use the BDFFileReader.dll
/*
/* Copyright: 2022 ELSYS AG
/* Programmer: Roman Bertschi
/*
/* Contact: email: info@elsys.ch
/*
// ********************************************************************************/
// -----------------------------------------------------------------------------------------------------------------
/*
* TpcAccess Example Program for ECR Measurement
*
* This short example shows how to use the TpcAccess.dll
* It shows how to build up a TPC_System, how to set recording parameter,
* how to redout the device status and how to redout the measurement data.
*
* ELSYS EXPRESSLY DISCLAIMS ALL WARRANITIES OR REPRESENTATIONS WITH RESPECT TO
* THIS SOFTWARE AND DOCUMENTATION AND ANY SUPPORT OR MAINTENANCE SERVICES THAT
* ELSYS MAY CHOOSE TO PROVIDE IN RELATION THERETO(INCLUDING, WITHOUT
* LIMITATION, ANY WARRANTIES OR CONDITIONS OF TITLE OR THAT THE SOFTWARE : WILL BE
* ERROR - FREE, WILL OPERATE WITHOUT INTERUPTION, WILL NOT INFRINGE THE RIGHTS OF A
* THIRD PARTY, OR WILL PRODUCE RESULTS IN CONNECTION WITH ITS USE).FURTHER,
* ELSYS EXPRESSLY DISCLAIMS ALL EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
*
* Copyright 2022 Elsys AG
* Author: Roman Bertschi
* Contact: info@elsys.ch
* Licence: MIT License
* See Licence file for more information about the used licence.
*
*/
// -----------------------------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <windows.h>
#include <stdio.h>
#include "bdfAPI.h"

using namespace std;
using namespace filereader;
int main()
{
    cout << "BDF File Reader Example!\n";

    // create an API Object  
    bdfAPI* api = CreateBDFAPIObj();

    // Open a BDF File for read access

    //int error = api->loadFile("..\\bdf\\heap0_Continuous-2s.bdf");
    //int error = api->loadFile("..\\bdf\\heap0_ECR-2-Channel-10-Blocks.bdf");
    int error = api->loadFile("..\\bdf\\heap0_ECR-2-Channel-10-Blocks-Dual.bdf");

     if (error == -1) {
        cout << "File could not been read!" << endl;
        return 0;
    }
    
     // Group = Cluster in TranAX, each group can run in a different measurement mode or with different sampling rate
    int nrOfGroups = api->getNumberOfGroups();     
    cout << "Number of Groups:\t " << nrOfGroups << endl;

    int nrOfInputs = 0;
    int nrOfBlocks = 0;

    // Limit readout buffer size
    const int MaxReadBuffer = 32 * 1024;
    double* data = new double[MaxReadBuffer];
    
    // loop through all blocks
    for (int g = 0; g < nrOfGroups; g++) {
        nrOfInputs = api->getNumberOfInputs(g);
        
        // asuming, all inputs have the same number of blocks, readout the number of blocks only from input 0
        nrOfBlocks = api->getNumberOfBlocks(g, 0);
        cout << "Group Nr: " << g << "\t Nr of Input: " << nrOfInputs << "\t Nr of Blocks:" << nrOfBlocks << endl;
        
        // Get some input related infos for data scaling 
        bdfAPI::sInputInfo inputInfo;
        for (int i = 0; i < nrOfInputs; i++) {
            api->getInputInfo(g, i, &inputInfo);
            cout << "Input " << i << " Analog Mask: " << inputInfo.AnalogMask << endl;
            cout << "Input " << i << " BinToPhysicalConstant: "      << inputInfo.BinToPhysicalConstant << endl;
            cout << "Input " << i << " BinToPhysicalFactor: "        << inputInfo.BinToPhysicalFactor << endl;
        }
        
        bdfAPI::sBlockInfo blockInfo;
        // get some block related infos for time information and sampling rate
        for (int block = 0; block < nrOfBlocks; block++) {
            api->getBlockInfo(g, 0, block, &blockInfo);
            cout << "Block " << block << " Length: " << blockInfo.BlockLength << endl;
            cout << "Block " << block << " Trigger Time: " << blockInfo.TriggerTimeSeconds << endl;
            cout << "Block " << block << " Trigger Sample: " << blockInfo.TriggerSample << endl;
            cout << "Block " << block << " Sampling Rate: " << blockInfo.SampleRateHertz/ 1000 / blockInfo.TimebaseDivisor << " kHz" << endl;

            int inputNr = 0;
            uint64_t StartReadAddr = 0;
            unsigned int ReadLength = blockInfo.BlockLength;
            if (blockInfo.BlockLength > MaxReadBuffer) ReadLength = MaxReadBuffer;

            // read out a chunk of data from the start of the block
            api->getDataD(g, inputNr, block, StartReadAddr, data, ReadLength);
            cout << "Data:" << data[0] << "\t" << data[1] << "\t" << data[2] << "\t" << data[3] << "..." << endl;
        }
       

    }
    delete[] data;

    api->closeFile();
    
    DestroyBDFAPIObj(api);

}
