#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"
#include <omp.h>

using namespace std;

#include "rtdsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  }
}


double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output){

  long long cycStart, cycStop;

  cycStart = rdtscll();

  short int input_h = input -> height - 1;
  short int input_w = input -> width - 1;
    
  output -> height = input_h + 1;
  output -> width = input_w + 1;
    
  short int filterIn[3][3];
  #pragma omp parallel for
  for(short int i = 0; i < 3; i++){
      filterIn[i][0] = filter -> get(i, 0);
      filterIn[i][1] = filter -> get(i, 1);
      filterIn[i][2] = filter -> get(i, 2);
  }
  
  // Gauss
  if(filterIn[0][1] == 4){
    #pragma omp parallel for
    for(short int p = 0; p < 3; p++){
      for(short int r = 1; r < input_h; r++){
        const short int r1 = r + 1;
        const short int r2 = r - 1;
        for(short int c = 1; c < input_w; c++){
          short int sums1 = 0;
          short int sums2 = 0;
          short int sums3 = 0; 
          
          const short int c1 = c + 1;
          const short int c2 = c - 1;
          
          sums2 += input -> color[p][r1][c] << 2; //(0,1)
          
          sums1 += input -> color[p][r][c1] << 2; //(1,0)
          sums2 += input -> color[p][r][c] << 3;   //(1,1)
          sums3 += input -> color[p][r][c1] << 2; //(1,2)
            
          sums2 += input -> color[p][r1][c] << 2; //(2,1)
            
          sums1 += sums2 + sums3;
          //Our divisor is 24, so we want to shift by 3(divide by 8) then divide by 3, 8 * 3 = 24:) 
          sums1 = (sums1 >> 3) / 3;
            
          sums1 = (sums1 < 0) ? 0 : sums1;
          sums1 = (sums1 > 255) ? 255 : sums1;
          
          output -> color[p][r][c] = sums1;
        }
      }      
    }
  }
    
  //Edge
  else if(filterIn[1][1] == -7){
    #pragma omp parallel for
    for(short int p = 0; p < 3; p++){
      for(short int r = 1; r < input_h; r++){
        const short int r1 = r + 1;
        const short int r2 = r - 1;
        for(short int c = 1; c < input_w; c++){
          short int sums1 = 0;
          short int sums2 = 0;
          short int sums3 = 0; 
          
          const short int c1 = c + 1;
          const short int c2 = c - 1;
          
          sums1 += input -> color[p][r2][c2]; //(0,0)
          sums2 += input -> color[p][r2][c]; //(0,1)
          sums3 += input -> color[p][r1][c]; //(0,2)
          
          sums1 += input -> color[p][r][c2]; //(1,0)
          sums2 += -((input -> color[p][r][c] << 2) + 3) ;   //(1,1)
          sums3 += -(input -> color[p][r][c1]); //(1,2)
           
          sums1 += input -> color[p][r1][c2]; //(2,0)
          sums2 += input -> color[p][r1][c]; //(2,1)
          sums3 += input -> color[p][r1][c1]; //(2,2)
          
          sums1 += sums2 + sums3;
          sums1 = (sums1 < 0) ? 0 : sums1;
          sums1 = (sums1 > 255) ? 255 : sums1;
          
          output -> color[p][r][c] = sums1;
        }
      }      
    }
  }
    
  //Ebmobss
  else if(filterIn[0][1] == 1 && filterIn[0][2] == -1){
    #pragma omp parallel for
    for(short int p = 0; p < 3; p++){
      for(short int r = 1; r < input_h; r++){
        const short int r1 = r + 1;
        const short int r2 = r - 1;
        for(short int c = 1; c < input_w; c++){
          short int sums1 = 0;
          short int sums2 = 0;
          short int sums3 = 0;
          
          const short int c1 = c + 1;
          const short int c2 = c - 1;
          
          sums1 += input -> color[p][r2][c2]; //(0,0)
          sums2 += input -> color[p][r2][c]; //(0,1)
          sums3 += -(input -> color[p][r1][c]); //(0,2)
          
          sums1 += input -> color[p][r][c2]; //(1,0)
          sums2 += input -> color[p][r][c] ;   //(1,1)
          sums3 += -(input -> color[p][r][c1]); //(1,2)
           
          sums1 += input -> color[p][r1][c2]; //(2,0)
          sums2 += -(input -> color[p][r1][c]); //(2,1)
          sums3 += -(input -> color[p][r1][c1]); //(2,2)
            
          sums1 += sums2 + sums3;
          sums1 = (sums1 < 0) ? 0 : sums1;
          sums1 = (sums1 > 255) ? 255 : sums1;
          
          output -> color[p][r][c] = sums1;
        }
      }      
    }
  }
                    
  //Hline
  else if(filterIn[0][1] == -2){
    #pragma omp parallel for
    for(short int p = 0; p < 3; p++){
      for(short int r = 1; r < input_h; r++){
        const short int r1 = r + 1;
        const short int r2 = r - 1;
        for(short int c = 1; c < input_w; c++){
          short int sums1 = 0;
          short int sums2 = 0;
          short int sums3 = 0;

          const short int c1 = c + 1;
          const short int c2 = c - 1;
          
          sums1 += -(input -> color[p][r2][c2]); //(0,0)
          sums2 += -(input -> color[p][r2][c] << 1); //(0,1)
          sums3 += -(input -> color[p][r1][c]); //(0,2)
           
          sums1 += input -> color[p][r1][c2]; //(2,0)
          sums2 += input -> color[p][r1][c] << 1; //(2,1)
          sums3 += input -> color[p][r1][c1]; //(2,2)
           
          sums1 += sums2 + sums3;
          sums1 = (sums1 < 0) ? 0 : sums1;
          sums1 = (sums1 > 255) ? 255 : sums1;
          
          output -> color[p][r][c] = sums1;
        }
      }      
    }
  }
  
  //Vline
  else if(filterIn[1][0] == -2){
    #pragma omp parallel for
    for(short int p = 0; p < 3; p++){
      for(short int r = 1; r < input_h; r++){
        const short int r1 = r + 1;
        const short int r2 = r - 1;
        for(short int c = 1; c < input_w; c++){
          short int sums1 = 0;
          short int sums2 = 0;
          short int sums3 = 0;

          const short int c1 = c + 1;
          const short int c2 = c - 1;
          
          sums1 += -(input -> color[p][r2][c2]); //(0,0)
          sums3 += input -> color[p][r1][c]; //(0,2)
          
          sums1 += -(input -> color[p][r][c2] << 1); //(1,0)
          sums3 += -(input -> color[p][r][c1] << 1); //(1,2)
           
          sums1 += -(input -> color[p][r1][c2]); //(2,0)
          sums3 += input -> color[p][r1][c1]; //(2,2)
            
          sums1 += sums1 + sums3;
          sums1 = (sums1 < 0) ? 0 : sums1;
          sums1 = (sums1 > 255) ? 255 : sums1;
          
          output -> color[p][r][c] = sums1;
        }
      }      
    }
  }
  
  //Sharpen
  else if(filterIn[0][0] == 11){
    #pragma omp parallel for
    for(short int p = 0; p < 3; p++){
      for(short int r = 1; r < input_h; r++){
        const short int r1 = r + 1;
        const short int r2 = r - 1;
        for(short int c = 1; c < input_w; c++){
          short int sums1 = 0;
          short int sums2 = 0;
          short int sums3 = 0;
          
          const short int c1 = c + 1;
          const short int c2 = c - 1;
          
          sums1 += (input -> color[p][r2][c2] << 3) + 3; //(0,0)
          sums2 += (input -> color[p][r2][c] << 3) + 2; //(0,1)
          sums3 += input -> color[p][r1][c]; //(0,2)
          
          sums1 += -(input -> color[p][r][c2]); //(1,0)
          sums2 += -(input ->color[p][r][c]);   //(1,1)
          sums3 += -(input ->color[p][r][c1]); //(1,2)
           
          sums1 += -(input -> color[p][r1][c2]); //(2,0)
          sums2 += -(input -> color[p][r1][c]); //(2,1)
          sums3 += -(input -> color[p][r1][c1]); //(2,2)
          
          sums1 += sums2 + sums3;      
          sums1 = sums1 / 20;
          sums1 = (sums1 < 0) ? 0 : sums1;
          sums1 = (sums1 > 255) ? 255 : sums1;
        
          output -> color[p][r][c] = sums1;
        }
      }      
    }
  }
  
  //average
  else{
    #pragma omp parallel for
    for(short int p = 0; p < 3; p++){
      for(short int r = 1; r < input_h; r++){
        const short int r1 = r + 1;
        const short int r2 = r - 1;
        for(short int c = 1; c < input_w; c++){
          short int sums1 = 0;
          short int sums2 = 0;
          short int sums3 = 0;
          
          const short int c1 = c + 1;
          const short int c2 = c - 1;
          
          sums1 += input -> color[p][r2][c2]; //(0,0)
          sums2 += input -> color[p][r2][c]; //(0,1)
          sums3 += input -> color[p][r1][c]; //(0,2)
          
          sums1 += input -> color[p][r][c2]; //(1,0)
          sums2 += input ->color[p][r][c];   //(1,1)
          sums3 += input ->color[p][r][c1]; //(1,2)
           
          sums1 += input -> color[p][r1][c2]; //(2,0)
          sums2 += input -> color[p][r1][c]; //(2,1)
          sums3 += input -> color[p][r1][c1]; //(2,2)
            
          sums1 += sums2 + sums3;
          sums1 = sums1 / 9;
          sums1 = (sums1 < 0) ? 0 : sums1;
          sums1 = (sums1 > 255) ? 255 : sums1;
          
          output -> color[p][r][c] = sums1;
        }
      }      
    }
  }              

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
  }
