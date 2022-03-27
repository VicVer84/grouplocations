#pragma once

#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>


using pattern = std::map<std::string, std::vector<std::string>>;
using newParams = std::map<std::string, std::vector<std::string>>;
using location = std::map<std::string, std::pair<std::vector<std::string>, pattern>>;
using locPair = std::pair<std::string, std::pair<std::vector<std::string>, std::map<std::string, std::vector<std::string>>>>;

std::ostream& PrintTimeNow(std::ostream& ofs);

void Log(const std::string& str);

void LTrim(std::string& s);
void RTrim(std::string& s);
void Trim(std::string& s);
bool StartsWith(std::string&& startsWith, std::string& str);

std::vector<std::string> GetLocationFiles(std::string& path);
pattern GetPatternsOrParams(std::string filename);
location ParseLocations(std::vector<std::string>& files);
void GroupWithK8sUpstreams(pattern& patterns, location& locations, std::unordered_set<std::string>& upstreams);
void Group(pattern& patterns, location& locations);

void SaveLocations(location& locations, std::string filename);
void SaveK8sMatchedLocationsToFile(const std::pair<std::string, std::vector<std::string>>& pattern, const location& locations, std::string filename);
void SaveMatchedLocationsToFile(const std::pair<std::string, std::vector<std::string>>& pattern, const location& locations, std::string filename);
void SaveUnmatchedLocationsToFile(const location& locations, std::string filename);

void CleanUp();
bool CheckForInclude(const std::string& original, const std::string& pattern);
std::vector<std::string> SplitIntoWords(const std::string& s);
std::unordered_set<std::string> GetK8sUpstreams(std::string filename);

void AddParamsToLocations(location& locations, const newParams& newParams);
void NewParamsSliceByLocationsAndSave(const newParams& newParams, std::string&& filename);