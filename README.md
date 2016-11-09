# AVR_SimpleSDAccess
A simple way to access SDCards from AVR mikrocontrollers. Based on [Roland Riegels Library](http://www.roland-riegel.de/sd-reader/).

Motivation: I searched through several libs to find one wich i can just use. Most of the ones i found needed another layer (MMC) to work.
Roland Riegels lib was close, so i used that, and implemented a few funtion to basically work with the SDCard as if you would work with the c FILE.

To use it, you just need to modify the ports and pins used for the SPI of the MC. Detailed info will follow.

Code to acces and write into file. This will overwirite an existing file or create a new one. Not existing directories will be crated.

```c
#include "sd_file_access.h"

int main(void){
    SD_FILE_t *file = sd_fopen("result.dot", OPEN_MODE_Wp)
    sd_fputcs(file, "HelloWorld\n");
    sd_fputcs(file, "KNOWN BUG: The last thing to write will be lost");
    sd_fclose(file);
}
```

Multiple files can be opened at the same time.
