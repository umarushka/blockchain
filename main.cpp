#include "BlockChain.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <sys/types.h>
#include <iostream>

using namespace std;

#include "errlog.h"
#include "timer.h"
#include "keyboard.h"
#include "BlockChainAddresses.h"

#pragma warning(disable:4996 4702 4505)

time_t zombieDate(0x510B56CB); // right around January 1, 2013

int getDir(string dir, vector<string> &files) {
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening directory: " << dir << endl;
        exit(0);
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

static double getMem() {

    char statFileName[256];
    sprintf(
            statFileName,
            "/proc/%d/statm",
            (int) getpid()
            );

    uint64_t mem = 0;
    FILE *f = fopen(statFileName, "r");
    if (1 != fscanf(f, "%" PRIu64, &mem)) {
        warning("coudln't read process size");
    }
    fclose(f);
    return (1e-9f * mem)*getpagesize();

}

enum StatResolution {
    SR_DAY,
    SR_MONTH,
    SR_YEAR,
    SR_LAST
};

enum CommandMode {
    CM_NONE, //
    CM_SCAN, // scanning.
    CM_PROCESS,
    CM_EXIT
};

class BlockChainCommand {
public:

    BlockChainCommand(const char *dataPath, string files) {
        info("Welcome to the BlockChain command parser.");
        info("Registered DataDirectory: %s to scan for the blockchain.", dataPath);
        mBlockChain = createBlockChain(dataPath, files); // Create the block-chain parser using this root path
        if (mBlockChain) {
            info("\e[32m File: %s opened successfuly in directory: %s \e[0m", files.c_str(), dataPath);
        } else {
            info("\e[31m Failed to open file: %s in directory: %s \e[0m", files.c_str(), dataPath);
            mMode = CM_EXIT;
        }
        mStatResolution = SR_YEAR;
        mMaxBlock = 500000;
        mProcessTransactions = false;
        mProcessBlock = 0;
        mLastBlockScan = 0;
        mLastBlockPrint = 0;
        mFinishedScanning = false;
        mCurrentBlock = NULL;
        mLastTime = 0;
        mSatoshiTime = 0;
        mMinBalance = 1;
        mRecordAddresses = false;
        mAddresses = NULL;
        mMode = CM_NONE;
    }

    ~BlockChainCommand(void) {
        if (mAddresses) {
            mAddresses->release();
        }
        if (mBlockChain) {
            mBlockChain->release();
        }
    }

    void readAndPrintBlockChain(uint32_t argc) {
        for (uint32_t i = 1; i < argc; i++) {
            uint32_t index = i;
            mCurrentBlock = getBlock(index);
            if (mCurrentBlock) {
                mBlockChain->printJSON(mCurrentBlock);
            }
        }
    }

    bool scanBlockChain() {
        bool ok = mBlockChain->readBlockHeaders(mMaxBlock, mLastBlockScan);
        if (!ok) {
            mFinishedScanning = true;
            mMode = CM_NONE; // done scanning.
            mLastBlockScan = mBlockChain->buildBlockChain();
            info("\e[44m Finished scanning block headers. Built block-chain with %d blocks found.. \e[0m", mLastBlockScan);
            readAndPrintBlockChain(mLastBlockScan);
            return false;
        }
        return true;
    }

    const BlockChain::Block *getBlock(uint32_t index) {
        mCurrentBlock = mBlockChain->readBlock(index);
        return mCurrentBlock;
    }

    CommandMode mMode;
    bool mRecordAddresses;
    bool mFinishedScanning;
    bool mProcessTransactions;
    StatResolution mStatResolution;
    uint32_t mProcessBlock;
    uint32_t mMaxBlock;
    uint32_t mLastBlockScan;
    uint32_t mLastBlockPrint;
    const BlockChain::Block *mCurrentBlock;
    BlockChain *mBlockChain;
    uint32_t mLastTime;
    uint32_t mSatoshiTime;
    uint32_t mMinBalance;
    BlockChainAddresses *mAddresses;
};

int main(int argc, const char **argv) {

    const char *dataPath = ".";
    vector<string> filesList;
    auto start = Timer::usecs();
    fprintf(stderr, "\n");
    info("\e[32m mem at start = %.3f Gigs \e[0m", getMem());

    if (argc < 2) {
        info("\e[31m Show the path to bitcoin blocks file.... \e[0m");
        exit(0);
    } else {
        dataPath = argv[1];
    }

    getDir(argv[1], filesList);
    std::sort(filesList.begin(), filesList.end());
    filesList.erase(filesList.begin());
    filesList.erase(filesList.begin());
    for (uint8_t i = 0; i < filesList.size(); ++i)
        cout << filesList[i] << endl;
    
    auto elapsed = Timer::usecs();
    for (uint32_t i = 0; i < filesList.size(); ++i) {
        BlockChainCommand bc(dataPath, filesList[i].c_str());
        while (bc.scanBlockChain());
        elapsed = (Timer::usecs() - start)*1e-6;
        info("\e[32m all done in %.2f seconds \e[0m", elapsed);
        info("\e[32m mem at end = %.3f Gigs \e[0m", getMem());
        info("\e[31m json file was output to current directory... \e[0m");
    }

    return 0;
}
