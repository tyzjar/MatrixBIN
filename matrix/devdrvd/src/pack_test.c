/* Packing Test Program */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "comm/devdrvd_communication.h"

int main()
{
  struct weight_struct test;
  struct timeval  tv;  

  //unsigned int time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;

  test.channel = 0;
  gettimeofday(&test.ts,  0);
  test.weight = 1000.0;
  test.tare = 500.0;
  test.status = 0;

  SET_BIT(test.status, TARE);
  SET_BIT(test.status, STABLE);

  char * data;

  int length = pack_weight(&data, &test);

  printf("Length of data=%d\n", length);
  printf("Data:%s\n", data);  

  // unpack test
  unpack_weight(&test, data);
  free(data);

  printf("** Unpack results **\n");
  printf("Channel=%d\n", test.channel);
  printf("Timestamp.sec=%d\n", test.ts.tv_sec);
  printf("Timestamp.usec=%d\n", test.ts.tv_usec);

  printf("Weight=%f\n", test.weight);
  printf("Tare=%f\n", test.tare);

  printf("Status=%d\n", test.status);

  return EXIT_SUCCESS;
}
