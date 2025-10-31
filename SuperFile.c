#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "General.h"
#include "FileHelper.h"
#include "SuperFile.h"
#include "Product.h"
#include "myMacros.h"


int	saveSuperMarketToFile(const SuperMarket* pMarket, const char* fileName, const char* customersFileName, int isCompressed)
{
	if (isCompressed)
	{
		return saveSuperMarketToFileComp(pMarket, fileName, customersFileName);
	}
	else
	{
		FILE* fp;
		fp = fopen(fileName, "wb");
		if (!fp)
		{
			printf("Error open supermarket file to write\n"); return 0;
		}

		if (!writeStringToFile(pMarket->name, fp, "Error write supermarket name\n"))
		{
			fclose(fp); return 0;
		}

		if (!writeIntToFile(pMarket->productCount, fp, "Error write product count\n"))
		{
			fclose(fp); return 0;
		}

		for (int i = 0; i < pMarket->productCount; i++)
		{
			if (!saveProductToFile(pMarket->productArr[i], fp))
			{
				fclose(fp); return 0;
			}
		}

		fclose(fp);

		saveCustomersToTextFile(pMarket->customerArr, pMarket->customerCount, customersFileName);

		return 1;
	}
}

int	loadSuperMarketFromFile(SuperMarket* pMarket, const char* fileName, const char* customersFileName, int isCompressed)
{
	if (isCompressed)
	{
		return loadSuperMarketFromFileComp(pMarket, fileName, customersFileName);
	}
	else
	{
		FILE* fp = fopen(fileName, "rb");
		if (!fp)
		{
			printf("Error open company file\n"); return 0;
		}

		pMarket->name = readStringFromFile(fp, "Error reading supermarket name\n");
		if (!pMarket->name)
		{
			fclose(fp); return 0;
		}

		int count;

		if (!readIntFromFile(&count, fp, "Error reading product count\n"))
		{

			FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
		}

		pMarket->productArr = (Product**)malloc(count * sizeof(Product*));
		if (!pMarket->productArr)
		{
			free(pMarket->name);
			fclose(fp); return 0;
		}

		pMarket->productCount = count;

		for (int i = 0; i < count; i++)
		{
			pMarket->productArr[i] = (Product*)malloc(sizeof(Product));
			if (!pMarket->productArr[i])
			{
				FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
			}

			if (!loadProductFromFile(pMarket->productArr[i], fp))
			{
				free(pMarket->productArr[i]);
				free(pMarket->name);
				fclose(fp); return 0;
			}
		}
		fclose(fp);

		pMarket->customerArr = loadCustomersFromTextFile(customersFileName, &pMarket->customerCount);
		if (!pMarket->customerArr) return 0;

		return	1;
	}
}

int	saveCustomersToTextFile(const Customer* customerArr, int customerCount, const char* customersFileName)
{
	FILE* fp;

	fp = fopen(customersFileName, "w");
	CHECK_MSG_RETURN_0(fp);

	fprintf(fp, "%d\n", customerCount);
	for (int i = 0; i < customerCount; i++)
		customerArr[i].vTable.saveToFile(&customerArr[i], fp);

	fclose(fp);
	return 1;
}

Customer* loadCustomersFromTextFile(const char* customersFileName, int* pCount)
{
	FILE* fp;

	fp = fopen(customersFileName, "r");
	CHECK_MSG_RETURN_0(fp);


	Customer* customerArr = NULL;
	int customerCount;

	fscanf(fp, "%d\n", &customerCount);

	if (customerCount > 0)
	{
		customerArr = (Customer*)calloc(customerCount, sizeof(Customer)); //cart will be NULL!!!
		if (!customerArr)
		{
			fclose(fp);
			return NULL;
		}

		for (int i = 0; i < customerCount; i++)
		{
			if (!loadCustomerFromFile(&customerArr[i], fp))
			{
				freeCustomerCloseFile(customerArr, i, fp);
				return NULL;
			}
		}
	}

	fclose(fp);
	*pCount = customerCount;
	return customerArr;
}

int	saveSuperMarketToFileComp(const SuperMarket* pMarket, const char* fileName, const char* customersFileName)
{
	FILE* fp = fopen(fileName, "wb");
	if (!fp)
		return 0;
	int len = strlen(pMarket->name);
	BYTE data[2] = { 0 };

	data[0] = pMarket->productCount >> 2;
	data[1] = (pMarket->productCount & 0x3) << 6;
	data[1] = data[1] | len;

	if (fwrite(data, sizeof(BYTE), 2, fp) != 2)
	{
		fclose(fp);
		return 0;
	}
	if (fwrite(pMarket->name, sizeof(char), len, fp) != len)
	{
		fclose(fp);
		return 0;
	}
	for (int i = 0; i < pMarket->productCount; i++)
	{
		if (!saveProductToFileComp(pMarket->productArr[i], fp))
		{
			fclose(fp);
			return 0;
		}
	}
	saveCustomersToTextFile(pMarket->customerArr, pMarket->customerCount, customersFileName);

	fclose(fp);
	return 1;
}


int loadSuperMarketFromFileComp(SuperMarket* pMarket, const char* fileName, const char* customersFileName)
{
	FILE* fp = fopen(fileName, "rb");
	if (!fp) return 0;

	BYTE countAndName[2];
	if (fread(countAndName, sizeof(BYTE), 2, fp) != 2)
	{
		CLOSE_RETURN_0(fp);
	}

	int lenName = countAndName[1] & 0x3F;  
	pMarket->productCount = (countAndName[0] << 2) | (countAndName[1] >> 6);

	pMarket->name = (char*)malloc(lenName + 1);  
	if (!pMarket->name)
	{
		CLOSE_RETURN_0(fp);
	}

	if (fread(pMarket->name, sizeof(char), lenName, fp) != lenName)
	{
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
	}
	pMarket->name[lenName] = '\0';  

	pMarket->productArr = malloc(pMarket->productCount * sizeof(Product*));
	if (!pMarket->productArr) 
	{
		CLOSE_RETURN_0(fp);
	}

	for (int i = 0; i < pMarket->productCount; i++)
	{
		pMarket->productArr[i] = malloc(sizeof(Product));
		if (!pMarket->productArr[i]) 
		{
			CLOSE_RETURN_0(fp);
		}
		if (!loadProductFromFileComp(pMarket->productArr[i], fp)) 
		{
			CLOSE_RETURN_0(fp);
		}
	}

	pMarket->customerArr = loadCustomersFromTextFile(customersFileName, &pMarket->customerCount);
	fclose(fp);
	return 1;
}

void freeCustomerCloseFile(Customer* customerArr, int count, FILE* fp)
{
	for (int i = 0; i < count; i++)
	{
		free(customerArr[i].name);
		customerArr[i].name = NULL;
		if (customerArr[i].pDerivedObj)
		{
			free(customerArr[i].pDerivedObj);
			customerArr[i].pDerivedObj = NULL;
		}
	}
	free(customerArr);
	fclose(fp);
}