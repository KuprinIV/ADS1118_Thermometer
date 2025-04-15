/*
 * print_to_string.h
 *
 *  Created on: 16 сент. 2023 г.
 *      Author: Ilya
 */

#ifndef INC_PRINT_TO_STRING_H_
#define INC_PRINT_TO_STRING_H_

#include <string.h>

void printFloat(char* res_str, const char* prestr, float* nums, const char **delims, int* precision, int size);
void printInteger(char* res_str, const char* prestr, int* nums, const char **delims, int size);
void printStrings(char* res_str, const char **strs, int size);
void printString(char* res_str, const char* prestr, const char* str);
int strLen(char* str);

#endif /* INC_PRINT_TO_STRING_H_ */
