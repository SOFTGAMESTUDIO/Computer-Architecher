#include <stdio.h>
#include <string.h>

void encryptDecrypt(char *data, const char *key, int encrypt)
{
    size_t dataLen = strlen(data);
    size_t keyLen = strlen(key);

    for (size_t i = 0; i < dataLen; i++)
    {
        unsigned char current = (unsigned char)data[i];
        unsigned char keyByte = (unsigned char)key[i % keyLen];

        if (encrypt)
        {
            data[i] = (char)((current ^ keyByte) + 1);
        }
        else
        {
            data[i] = (char)(((unsigned char)(current - 1)) ^ keyByte);
        }
    }
}

int main()
{
    char data[] = "Hello World";
    char key[] = "H3#sd@df3##HSTEH116^28836^^@*#%@*";

    printf("Original: %s\n", data);

    encryptDecrypt(data, key, 1);
    printf("Encrypted: %s\n", data);

    encryptDecrypt(data, key, 0);
    printf("Decrypted: %s\n", data);

    return 0;
}