/* PROFILING */
gcc -g -pg -no-pie -o profiling.exe main.c
run executable
gprof profiling.exe > profile.txt

/* TO PROPERLY DISPLAY UNICODE IN WINDOWS CONSOLES */
CHCP 65001

/* TEST INPUT */
γειά σου κόσμος
£来. Testing unicode -- English -- Ελληνικά -- Español.

/* Evil bit shifting */

// from bytes
uint8_t bytes[8];
uint64_t big = 65535;

memcpy(&bytes[0], &big, sizeof( uint64_t ) );
memcpy(&big, &bytes[0], sizeof( uint64_t ) );

uint64_t res = bytes[7];
res = (res << 8) | bytes[6];
res = (res << 8) | bytes[5];
res = (res << 8) | bytes[4];
res = (res << 8) | bytes[3];
res = (res << 8) | bytes[2];
res = (res << 8) | bytes[1];
res = (res << 8) | bytes[0];

// to bytes
uint64_t val = 0xFFFFFFFFFFFFFFFF;

uint8_t byte1 = (val >> 56) & 0xFFFFFFFFFFFFFFFF; 
uint8_t byte2 = (val >> 48) & 0xFFFFFFFFFFFFFFFF; 
uint8_t byte3 = (val >> 40) & 0xFFFFFFFFFFFFFFFF; 
uint8_t byte4 = (val >> 32) & 0xFFFFFFFFFFFFFFFF; 
uint8_t byte5 = (val >> 24) & 0xFFFFFFFFFFFFFFFF; 
uint8_t byte6 = (val >> 16) & 0xFFFFFFFFFFFFFFFF; 
uint8_t byte7 = (val >> 8) & 0xFFFFFFFFFFFFFFFF; 
uint8_t byte8 = val & 0xFFFFFFFFFFFFFFFF;