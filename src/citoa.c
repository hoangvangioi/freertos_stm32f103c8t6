#include "citoa.h"

void reverse(char str[], int length)
{
	int start = 0;
	int end	  = length - 1;
	while (start < end)
	{
		char temp  = str[start];
		str[start] = str[end];
		str[end]   = temp;
		end--;
		start++;
	}
}

char *citoa(int num, char *str, int base)
{
	int	 i			= 0;
	bool isNegative = false;

	if (num == 0)
	{
		str[i++] = '0';
		str[i]	 = '\0';
		return str;
	}

	if (num < 0 && base == 10)
	{
		isNegative = true;
		num		   = -num;
	}

	while (num != 0)
	{
		int rem	 = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num		 = num / base;
	}

	if (isNegative)
		str[i++] = '-';

	str[i] = '\0';

	reverse(str, i);

	return str;
}

// int main()
// {
// 	char str[100];
// 	printf("Number: %d\nBase: %d\tConverted String: %s\n", 1567, 10, citoa(1567, str, 10));
// 	printf("Base: %d\t\tConverted String: %s\n", 2, citoa(1567, str, 2));
// 	printf("Base: %d\t\tConverted String: %s\n", 8, citoa(1567, str, 8));
// 	printf("Base: %d\tConverted String: %s\n", 16, citoa(1567, str, 16));
// 	return 0;
// }