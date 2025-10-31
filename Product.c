#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "Product.h"
#include "General.h"
#include "FileHelper.h"
#include "SuperFile.h"

#define MIN_DIG 3
#define MAX_DIG 5

static const char* typeStr[eNofProductType] = { "Fruit Vegtable", "Fridge", "Frozen", "Shelf" };
static const char* typePrefix[eNofProductType] = { "FV", "FR", "FZ", "SH" };


void	initProduct(Product* pProduct)
{
	initProductNoBarcode(pProduct);
	generateBarcode(pProduct);
}

void	initProductNoBarcode(Product* pProduct)
{
	initProductName(pProduct);
	pProduct->type = getProductType();
	initDate(&pProduct->expiryDate);
	pProduct->price = getPositiveFloat("Enter product price\t");
	pProduct->count = getPositiveInt("Enter product number of items\t");
}

void initProductName(Product* pProduct)
{
	do {
		printf("enter product name up to %d chars\n", NAME_LENGTH);
		myGets(pProduct->name, sizeof(pProduct->name), stdin);
	} while (checkEmptyString(pProduct->name));
}

int	saveProductToFile(const Product* pProduct, FILE* fp)
{
	if (fwrite(pProduct, sizeof(Product), 1, fp) != 1)
	{
		puts("Error saving product to file\n");
		return 0;
	}
	return 1;
}

int	loadProductFromFile(Product* pProduct, FILE* fp)
{
	if (fread(pProduct, sizeof(Product), 1, fp) != 1)
	{
		puts("Error reading product from file\n");
		return 0;
	}
	return 1;
}


int saveProductToFileComp(const Product* pProduct, FILE* fp)
{
	BYTE data[4] = { 0 };
	int number, j = 0;
	for (int i = 2; i < BARCODE_LENGTH - 1; i += 2)
	{
		number = pProduct->barcode[i] - '0';
		data[j] = number << 4;
		number = pProduct->barcode[i + 1] - '0';
		data[j] = data[j] | number;
		j++;
	}
	number = pProduct->barcode[7] - '0';
	data[2] = number << 4;
	data[2] |= (pProduct->type << 2);
	int len = strlen(pProduct->name);
	data[2] = data[2] | len >> 2;
	data[3] = len << 6;
	if (fwrite(data, sizeof(BYTE), 4, fp) != 4)
		return 0;
	if (fwrite(pProduct->name, sizeof(char), len, fp) != len)
		return 0;
	if (!saveCountPriceToBinaryFileCompressed(pProduct, fp))
		return 0;
	if (!saveDateToBinaryFileCompressed(&pProduct->expiryDate, fp))
		return 0;
	return 1;
}

int saveCountPriceToBinaryFileCompressed(const Product* pProduct, FILE* fp)
{
	BYTE data[3] = { 0 };
	data[0] = pProduct->count;
	int cents, priceWhole = pProduct->price * 100;
	cents = priceWhole % 100;
	priceWhole /= 100;
	data[1] = (cents << 1) | (priceWhole >> 8);
	data[2] = priceWhole & 0xFF;
	if (fwrite(data, sizeof(BYTE), 3, fp) != 3)
		return 0;
	return 1;
}

int saveDateToBinaryFileCompressed(const Date* pDate, FILE* fp)
{
	BYTE data[2] = { 0 };
	data[0] = pDate->day << 3;
	data[0] = data[0] | (pDate->month >> 1);
	int year = pDate->year - 2024;
	data[1] = pDate->month << 7 | (year & 0x07) << 4;
	if (fwrite(data, sizeof(BYTE), 2, fp) != 2) return 0;
	return 1;
}


int loadProductFromFileComp(Product* pProduct, FILE* fp)
{
	BYTE nameBarcode[4], countPrice[3], date[2];

	if (fread(nameBarcode, sizeof(BYTE), 4, fp) != 4) return 0;

	pProduct->type = (nameBarcode[2] >> 2) & 0x3;
	if (pProduct->type < 0 || pProduct->type >= eNofProductType) return 0;
	const char* typePrefix = getProductTypePrefix(pProduct->type);

	pProduct->barcode[0] = typePrefix[0];
	pProduct->barcode[1] = typePrefix[1];


	int j = 0, i;
	for (i = 2; i < 6; i += 2)
	{
		pProduct->barcode[i] = ((nameBarcode[j] >> 4) & 0xf) + '0'; 
		pProduct->barcode[i + 1] = (nameBarcode[j] & 0xf) + '0';
		j++;
	}
	pProduct->barcode[i] = ((nameBarcode[2] >> 4) & 0xf) + '0';
	pProduct->barcode[7] = '\0';


	int lenName = ((nameBarcode[2] & 0x03) << 2)| ((nameBarcode[3] >> 6) & 0x03);
	if (lenName > NAME_LENGTH) return 0;
	if (fread(pProduct->name, sizeof(char), lenName, fp) != lenName) return 0;
	pProduct->name[lenName] = '\0';


	if (fread(countPrice, sizeof(BYTE), 3, fp) != 3) return 0;

	pProduct->count = countPrice[0];
	int penny = countPrice[1] >> 1;
	int noPenny = countPrice[1] >> 7 | countPrice[2];
	pProduct->price = (penny / 100.0f) + noPenny;

	if (fread(date, sizeof(BYTE), 2, fp) != 2) return 0;

	pProduct->expiryDate.day = (date[0] >> 3);
	pProduct->expiryDate.month = ((date[0] & 0x07) << 1) | (date[1] >> 7);
	pProduct->expiryDate.year = (date[1] >> 4) & 0x07;

	return 1;
}

int	compareProductsByName(const void* prod1, const void* prod2)
{
	const Product* pProd1 = *(Product**)prod1;
	const Product* pProd2 = *(Product**)prod2;

	return strcmp(pProd1->name, pProd2->name);
}

int	compareProductsByCount(const void* prod1, const void* prod2)
{
	const Product* pProd1 = *(Product**)prod1;
	const Product* pProd2 = *(Product**)prod2;

	return pProd1->count - pProd2->count;
}

int	compareProductsByPrice(const void* prod1, const void* prod2)
{
	const Product* pProd1 = *(Product**)prod1;
	const Product* pProd2 = *(Product**)prod2;

	if (pProd1->price > pProd2->price)
		return 1;
	if (pProd1->price < pProd2->price)
		return -1;
	return 0;
}

void printProduct(const Product* pProduct)
{
	char* dateStr = getDateStr(&pProduct->expiryDate);
	printf("%-20s %-10s\t", pProduct->name, pProduct->barcode);
	printf("%-20s %5.2f %13d %7s %15s\n", typeStr[pProduct->type], pProduct->price, pProduct->count, " ", dateStr);
	free(dateStr);
}

void printProductPtr(void* v1)
{
	Product* pProduct = *(Product**)v1;
	printProduct(pProduct);
}

void generateBarcode(Product* pProd)
{
	char temp[BARCODE_LENGTH + 1];
	int barcodeNum;

	strcpy(temp, getProductTypePrefix(pProd->type));
	do {
		barcodeNum = MIN_BARCODE + rand() % (RAND_MAX - MIN_BARCODE + 1); //Minimum 5 digits
	} while (barcodeNum > MAX_BARCODE);
	
	sprintf(temp + strlen(temp), "%d", barcodeNum);

	strcpy(pProd->barcode, temp);
}

void getBarcodeCode(char* code)
{
	char temp[MAX_STR_LEN];
	char msg[MAX_STR_LEN];
	sprintf(msg, "Code should be of %d length exactly\n"
				 "Must have %d type prefix letters followed by a %d digits number\n"
				 "For example: FR20301",
				 BARCODE_LENGTH, PREFIX_LENGTH, BARCODE_DIGITS_LENGTH);
	int ok = 1;
	int digCount = 0;
	do {
		ok = 1;
		digCount = 0;
		getsStrFixSize(temp, MAX_STR_LEN, msg);
		if (strlen(temp) != BARCODE_LENGTH)
		{
			puts("Invalid barcode length");
			ok = 0;
		}
		else
		{
			//check first PREFIX_LENGTH letters are upper case and valid type prefix
			char* typeSubStr = (char*)malloc(PREFIX_LENGTH + 1);
			if (!typeSubStr)
				return;
			strncpy(typeSubStr, temp, PREFIX_LENGTH);
			typeSubStr[PREFIX_LENGTH] = '\0';
			int prefixOk = 0;
			int i;

			for (i = 0; i < eNofProductType; i++)
			{
				if (strcmp(typeSubStr, typePrefix[i]) == 0)
				{
					prefixOk = 1;
					break; //found valid type prefix
				}
			}

			free(typeSubStr); //free the allocated memory

			if (!prefixOk)
			{
				puts("Invalid type prefix");
				ok = 0;
			}
			else
			{
				for (i = PREFIX_LENGTH; i < BARCODE_LENGTH; i++)
				{
					if (!isdigit(temp[i]))
					{
						puts("Only digits after type prefix\n");
						puts(msg);
						ok = 0;
						break;
					}
					digCount++;
				}

				if (digCount != BARCODE_DIGITS_LENGTH)
				{
					puts("Incorrect number of digits");
					ok = 0;
				}
			}
		}
	} while (!ok);

	strcpy(code, temp);
}

eProductType getProductType()
{
	int option;

	printf("\n");
	do {
		printf("Please enter one of the following types\n");
		for (int i = 0; i < eNofProductType; i++)
			printf("%d for %s\n", i, typeStr[i]);
		scanf("%d", &option);
	} while (option < 0 || option >= eNofProductType);

	getchar();

	return (eProductType)option;
}

const char* getProductTypeStr(eProductType type)
{
	if (type < 0 || type >= eNofProductType)
		return NULL;
	return typeStr[type];
}

const char* getProductTypePrefix(eProductType type)
{
	if (type < 0 || type >= eNofProductType)
		return NULL;
	return typePrefix[type];
}

int isProduct(const Product* pProduct, const char* barcode)
{
	if (strcmp(pProduct->barcode, barcode) == 0)
		return 1;
	return 0;
}

void updateProductCount(Product* pProduct)
{
	int count;

	do {
		printf("How many items to add to stock? ");
		scanf("%d", &count);
	} while (count < 1);

	pProduct->count += count;
}


void	freeProduct(Product* pProduct)
{
	//nothing to free!!!!
}