#include "Utils.h"

#include <iostream>
#include <string>


int main(int argc, char** argv) {
    std::vector<std::string> allArgs(argv, argv + argc);

    std::string locationsPath = ".\\locations\\";
    std::string patternPath = "patterns.txt";
    std::string newParamsFile = "NewParams.txt";

    if (allArgs.size() > 1) {
       locationsPath = allArgs[1];
    }

    if (allArgs.size() > 2) {
        std::string patternPath = allArgs[2];
    }

    CleanUp();

    std::vector<std::string> v = GetLocationFiles(locationsPath);
    std::cout << "Files count: " << v.size() << std::endl;
    
    location locations = ParseLocations(v);
    std::cout << "Locations found: " << locations.size() << std::endl;

    pattern patterns = GetPatternsOrParams(patternPath);
    std::cout << "Patterns found: " << patterns.size() << std::endl;    

    newParams newParams = GetPatternsOrParams(newParamsFile);

    AddParamsToLocations(locations, newParams);
    NewParamsSliceByLocationsAndSave(newParams, "NewParamsSliceByLocations.txt");

    SaveLocations(locations, "LocationsList.txt");

    std::unordered_set<std::string> upstreams = GetK8sUpstreams("upstreams.txt");
    std::cout << "k8s upstreams found: " << upstreams.size() << std::endl;

    GroupWithK8sUpstreams(patterns, locations, upstreams);

    Group(patterns, locations);
    SaveUnmatchedLocationsToFile(locations, "UnmatchedLocations.txt");

}


