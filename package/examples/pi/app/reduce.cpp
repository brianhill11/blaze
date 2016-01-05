#include <ctype.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <string>

#include <glog/logging.h>

#include "Client.h"

using namespace blaze;

class PiClient: public Client {
public:

  PiClient(): Client("Pi") {;}

  void compute() {
    double* data_ptr = (double*)getData(0);
    uint32_t* output_ptr = (uint32_t*)getData(1);

    uint32_t inside_count = 0;
    uint32_t outside_count = 0;

    for (int i=0; i<getInputNumItems(0); i++) {
      double x = data_ptr[i*2+0];
      double y = data_ptr[i*2+1];

      if (x*x+y*y < 1.0) {
        inside_count++;
      }
      else {
        outside_count++;
      }
    }
    output_ptr[0] = inside_count;
    output_ptr[1] = outside_count;
  } 
};

int main(int argc, char** argv) {
 
  // GLOG configuration
  FLAGS_logtostderr = 1;
  FLAGS_v = 0;

  std::vector<std::pair<double, double> > all_data;
  for (std::string line; std::getline(std::cin, line); ) 
  {
    std::stringstream ss(line);

    std::string key;
    std::string value;
    double x = 0;
    double y = 0;

    std::getline(ss, key, '\t');

    try {
      std::getline(ss, value, ',');
      x = std::stof(value);
      std::getline(ss, value);
      y = std::stof(value);
    } catch (std::exception &e) {
      continue; 
    }
    all_data.push_back(std::make_pair(x, y));
  }

  // send to accelerator
  PiClient client;

  double* data_ptr = (double*)client.alloc(all_data.size(), 2, 2*sizeof(double), BLAZE_INPUT);
  unsigned int* output_ptr = (unsigned int*)client.alloc(2, 1, 1*sizeof(unsigned int), BLAZE_OUTPUT);

  for (int i=0; i<all_data.size(); i++) {
    data_ptr[i*2+0] = all_data[i].first;
    data_ptr[i*2+1] = all_data[i].second;
  }

  // start computation
  client.start();

  printf("0\t%d\n", output_ptr[0]);
  printf("1\t%d\n", output_ptr[1]);

  return 0;
}
