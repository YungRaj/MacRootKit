#include "strparse.hpp"

#ifdef __KERNEL__

int isspace(int c)
{
	return (c == '\t' || c == '\n' ||
	    c == '\v' || c == '\f' || c == '\r' || c == ' ' ? 1 : 0);
}

#endif

static uint32_t log2(uint64_t value)
{
	uint32_t shift = 0;

	while(value >>= 1) ++shift;

	return shift;
}

int hex_digit(char ch)
{
	if('0' <= ch && '9' >= ch)
		return ch - '0';
	else if('A' <= ch && 'F' >= ch)
		return ch - 'A' + 0xa;
	else if('a' <= ch && 'f' >= ch)
		return ch - 'a' + 0xa;

	return -1;
}

char* strnchar(char *str, uint32_t len, char ch)
{
	char *s = str;
	char *end = str + len;

	while(s < end)
	{
		if(*s == ch)
			return s;

		if(*s == 0)
			return NULL;
		
		s++;
	}

	return NULL;
}

enum strtoint_result strtoint(char *str,
							  uint32_t len,
							  bool sign,
							  bool is_signed,
							  uint32_t base,
							  uint64_t *value,
							  char **end)
{
	enum strtoint_result result = STRTOINT_OK;

	char *last = str + len;

	bool negate = false;

	uint64_t _value = 0;

	if(last == str)
		goto no_chars;

	if(sign && (str[0] == '+' || str[0] == '-'))
	{
		negate = (str[0] == '-');
		str++;
	} else if(str[0] == '0'
			  && (str[1] == 'x' || str[1] == 'o' || str[1] == 'b'))
	{
		if(str[1] == 'x')
			base = 16;
		if(str[1] == 'o')
			base = 8;
		if(str[1] == 'b')
			base = 2;

		str += 2;
	}

	int d;

	if(last == str)
		goto no_chars;

	d = hex_digit(*str);

	if(d < 0 || d >= base)
		goto no_chars;

	while(str != last && *str != 0)
	{
		d = hex_digit(*str);

		if(d < 0 || d >= base)
		{
			result = STRTOINT_BADDIGIT;

			goto fail;
		}

		uint64_t new_value = _value * base + d;

		if(is_signed)
		{
			uint64_t max = (uint64_t) (negate ? INTMAX_MIN : INTMAX_MAX);

			if(new_value > max)
			{
				result = STRTOINT_OVERFLOW;

				goto fail;
			}
		} else if(new_value < _value)
		{
			result = STRTOINT_OVERFLOW;

			goto fail;
		}

		_value = new_value;
		str++;
	}

	if(negate)
		_value = (uint64_t)(-(int64_t)_value);

	*value = _value;

	*end = str;

	return result;

no_chars:
	result = STRTOINT_NODIGITS;
fail:
	*end = str;
	
	return result;

}

enum strtodata_result strtodata(char *str,
								uint32_t base,
								void *data,
								uint32_t *size,
								char **end)
{
	enum strtodata_result result = STRTODATA_OK;

	char *start = str;

	if(str[0] == '0'
	   && (str[1] == 'x' || str[1] == 'o' || str[1] == 'b'))
	{
		if(str[1] == 'x')
			base = 16;
		if(str[1] == 'o')
			base = 8;
		if(str[1] == 'b')
			base = 2;

		str += 2;
	}

	uint32_t bits_per_digit = log2(base);

	uint8_t *p = (uint8_t*) data;

	uint32_t left = (p == NULL ? 0 : *size);

	uint32_t realsize = 0;

	do
	{
		uint8_t byte = 0;

		for(uint32_t i = 0; i < 8 / bits_per_digit; i++) 
		{
			int d = hex_digit(*str);
			
			if( d < 0 || d >= base)
			{
				if(i == 0)
				{
					if(str == start)
						result = STRTODATA_NODIGITS;
					else
						result = STRTODATA_BADDIGIT;

					goto no_digits;
				}

				result = STRTODATA_NEEDDIGIT;

				goto fail;
			}

			byte |= d << (8 - (i + 1) * bits_per_digit);
			
			str++;
		}

		realsize++;

		if(left > 0)
		{
			*p = byte;
			p++;

			left--;
		}
	} while(*str != 0);

no_digits:
	*size = realsize;

fail:
	*end = str;

	return result;
}

enum strparse_result strreplace(char *str, char find, char replace)
{
	int digits = 0;

	for(int i = 0; i < strlen(str); i++)
	{
		if(str[i] == find)
		{
			str[i] = replace;

			digits++;
		}
	}

	if(!digits)
		return STRPARSE_NODIGITS;

	return STRPARSE_OK;
}

#ifdef __KERNEL__

char* strdup(char *s)
{
	size_t l;
	char *t;

	if (s == NULL) return NULL;
	
	l = strlen(s);
	t = new char[l + 1];
	
	memcpy(t, s, l);
	
	t[l] = '\0';
	
	return t;
}

char* strstr(char *string, char *substring)
{
	char *a, *b;

	/* First scan quickly through the two strings looking for a
	 * single-character match.  When it's found, then compare the
	 * rest of the substring.
	 */

	b = substring;

	if (*b == 0)
	{
		return string;
	}

	for ( ; *string != 0; string += 1)
	{
		if (*string != *b) {
		 	continue;
		}
		
		a = string;
		
		while (1)
		{
			if (*b == 0)
			{
				return string;
			}

			if (*a++ != *b++)
			{
				break;
			}
		}

		b = substring;
	}

	return NULL;
}

#endif

char* strtokmul(char *input, char *delimiter)
{
	static char *string;
	
	if(input != NULL)
		string = input;

	if(string == NULL)
		return string;

	char *end = strstr(string, delimiter);
	
	if(end == NULL)
	{
		char *temp = string;
		
		string = NULL;
		
		return temp;
	}

	char *temp = string;
	
	*end = '\0';
	
	string = end + strlen(delimiter);
	
	return temp;
}

char* ltrim(char *s)
{
	while(isspace(*s)) s++;

	return s;
}

char* rtrim(char *s)
{
	char* back = s + strlen(s);

	while(isspace(*--back));

	*(back + 1) = '\0';

	return s;
}

char* trim(char *s)
{
	return rtrim(ltrim(s)); 
}

char* deblank(char* input)
{
	int i, j;

	char *output = input;
	
	for (i = 0, j = 0; i < strlen(input); i++, j++)
	{
		if (input[i] != ' ')
			output[j] = input[i];
		else
			j--;
	}

	output[j] = '\0';

	return output;
}

/*
#include <string.h> 

int main()
{
	uint64_t value;

	char *s = "0xfffffff007b6b668";
	char *end = NULL;

	uint8_t *data;
	uint32_t data_len;

	enum strtoint_result sir;
	enum strtodata_result sdr;

	sdr = strtodata(s, 16, NULL, &data_len, &end);
	
	data = malloc(data_len);

	printf("data_len = 0x%x\n", data_len);

	sdr = strtodata(s, 16, data, &data_len, &end);

	for(uint32_t i = 0; i < data_len; i++)
		printf("%x", data[i]);

	printf("\n");

	printf("0x%llx\n", *(uint64_t*) data);

	sir = strtoint(s, strlen(s), true, false, 16, &value, &end);

	printf("0x%llx\n", value);
}
*/