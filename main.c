#include <string.h>
#include "main.h"

void main(int argc, char** argv, char**env_var_ptr) {
  // Check number of arguments
  if (argc < 2) {
    printf("Not enough arguments.\n");
    return;
  }
  
  // Parse argv[1] for device name
  size_t len = strlen(argv[1]);
  char* device = malloc(len+1);
  strcpy(device, argv[1]);

  // Check device name length
  if (len != 8) {
    printf("Invalid device name\n");
    free(device);
    return;
  }
  
  // Read parsed device
  int fd  = open(device, O_RDONLY);

  // Validate device check
  if (fd <= 0)
    printf("\tValue is less than 0\n\tValue is: %d\n\n", fd);
  else {
    // Call the function from Project 1 to get the address of MFT
    findjpg(fd);
  }

  // Free up memory from copied string
  free(device);
  return;
}
