#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <pthread.h>
#include "utility.h"
#include "star.h"
#include "float.h"

#define NUM_STARS 30000
#define MAX_LINE 1024
#define DELIMITER " \t\n"

struct Star star_array[NUM_STARS];
uint8_t (*distance_calculated)[NUM_STARS];

double min = FLT_MAX;
double max = FLT_MIN;
double distance;

double avg = 0;
uint32_t thds = 1;
uint32_t ct = 0;
pthread_mutex_t mutex;

void showHelp() 
{
  printf("Use: findAngular [options]\n");
  printf("Where options are:\n");
  printf("-t           Number of threads to use\n");
  printf("-h           Show this help\n");
}

void *determineAverageAngularDistance(void *ptr) 
{
  double mean = 0;

  uint32_t i, j;
  uint64_t count = 0;

  double temp_min = FLT_MAX;
  double temp_max = FLT_MIN;
  uint32_t start_thd = (NUM_STARS / thds) * ct;
  uint32_t end_thd;

  if(ct == (thds - 1))
  {
    end_thd = NUM_STARS;
  }
  else
  {
    end_thd = (NUM_STARS / thds) * (ct + 1);
  }

  for (i = start_thd; i < end_thd; i++) 
  {
    for (j = 0; j < NUM_STARS; j++) 
    {
      if (i != j && distance_calculated[i][j] == 0) 
      {
        double distance = ( star_array[i].RightAscension, star_array[j].Declination,
                           star_array[j].RightAscension, star_array[j].Declination );
        distance_calculated[i][j] = 1;
        distance_calculated[j][i] = 1;
        count++;

        if (temp_min > distance) 
        {
          temp_min = distance;
        }

        if (temp_max < distance) {
          temp_max = distance;
        }
        mean = mean + (distance - mean) / count;
      }
    }
  }
  pthread_mutex_lock(&mutex);
  if (min > temp_min) 
  {
    min = temp_min;
  }
  if (max < temp_max) 
  {
    max = temp_max;
  }
  avg = avg + (mean - avg);
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main( int argc, char *argv[] ) 
{
  FILE *fp;
  uint32_t star_count = 0;

  clock_t start, end;
  double cpu_time_used;
  uint32_t n, numt;

  for (numt = 1; numt < argc; numt++) 
  {
    if (strcmp(argv[numt], "-t") == 0) 
    {
      thds = atoi(argv[numt + 1]);
    }
  }

  distance_calculated = malloc(sizeof(uint8_t[NUM_STARS][NUM_STARS]));

  if( distance_calculated == NULL )
  {
    uint64_t num_stars = NUM_STARS;
    uint64_t size = num_stars * num_stars * sizeof(uint8_t);
    printf("Could not allocate %ld bytes\n", size);
    exit( EXIT_FAILURE );
  }

  memset(distance_calculated, 0, sizeof(uint8_t[NUM_STARS][NUM_STARS]));

  for ( n = 1; n < argc; n++ ) 
  {
    if ( strcmp(argv[n], "-help" ) == 0 ) 
    {
      showHelp();
      exit(0);
    }
  }

  fp = fopen( "data/tycho-trimmed.csv", "r" );

  if ( fp == NULL ) 
  {
    printf("ERROR: Unable to open the file data/tycho-trimmed.csv\n"); 
    exit(1); 
  }

  char line[MAX_LINE];
  while (fgets(line, 1024, fp))
  {
    uint32_t column = 0;

    char* tok;
    for (tok = strtok(line, " ");
            tok && *tok;
            tok = strtok(NULL, " "))
    {
       switch( column )
       {
          case 0:
              star_array[star_count].ID = atoi(tok);
              break;
       
          case 1:
              star_array[star_count].RightAscension = atof(tok);
              break;
       
          case 2:
              star_array[star_count].Declination = atof(tok);
              break;

          default: 
             printf("ERROR: line %d had more than 3 columns\n", star_count );
             exit(1);
             break;
       }
       column++;
    }
    star_count++;
  }
  printf("%d records read\n", star_count );

  start = clock();
  pthread_t thread[thds];
  uint32_t pt;

  for (pt = 0; pt < thds; pt++) 
  { 
    ct = pt;
    if(pthread_create(&thread[pt], NULL, determineAverageAngularDistance, NULL))
    {
      perror("Error creating thread: ");
      exit( EXIT_FAILURE ); 
    }
  }
  for (pt = 0; pt < thds; pt++) 
  { 
    if(pthread_join(thread[pt], NULL))
    {
      perror("Problem with pthread_join: ");
    } 
  }

  end = clock(); 
  cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("Average angular separation: %lf\n", avg); 
  printf("Minimum angular separation: %lf\n", min); 
  printf("Maximum angular separation: %lf\n", max); 
  printf("Time taken to calculate: %lf seconds\n", cpu_time_used);

  return 0; 
}
