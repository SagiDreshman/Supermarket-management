#pragma once

#include <stdio.h>
#include "Supermarket.h"
#include "Customer.h"

typedef unsigned char BYTE;


//text file
int	saveCustomersToTextFile(const Customer* customerArr, int customerCount, const char* customersFileName);
Customer* loadCustomersFromTextFile(const char* customersFileName, int* pCount);
void freeCustomerCloseFile(Customer* customerArr, int customerIndex, FILE* fp);

// binary files
int	saveSuperMarketToFile(const SuperMarket* pMarket, const char* fileName,const char* customersFileName, int isCompressed);
int	loadSuperMarketFromFile(SuperMarket* pMarket, const char* fileName,const char* customersFileName, int isCompressed);

//binary files compress
int saveSuperMarketToFileComp(const SuperMarket* pMarket, const char* fileName, const char* customersFileName);
int loadSuperMarketFromFileComp(SuperMarket* pMarket, const char* fileName, const char* customersFileName);
