#include "Utils.h"

void LTrim(std::string& s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) {
		return !std::iswspace(ch);
		}));
}

void RTrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::iswspace(ch);
		}).base(), s.end());
}

void Trim(std::string& s) {
	LTrim(s);
	RTrim(s);
}

bool StartsWith(std::string&& startsWith, std::string& str) {
	Trim(str);
	return str.rfind(startsWith, 0) == 0;
}

std::ostream& PrintTimeNow(std::ostream& ofs) {
	time_t rawtime;
	struct tm* timeinfo = new tm;
	char buffer[80];

	time(&rawtime);
	localtime_s(timeinfo, &rawtime);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
	delete timeinfo;

	return ofs << buffer << ": ";
}

void Log(const std::string& str) {
	PrintTimeNow(std::cout) << str << std::endl;
}

std::vector<std::string> GetLocationFiles(std::string& path) {
	std::vector<std::string> names;

	if (std::filesystem::exists(path)) {
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			std::stringstream ss;
			ss << entry.path();
			std::string str;
			std::getline(ss, str);

			for (size_t i = 0; i < str.size(); ++i) {
				if (str[i] == '"') {
					str.erase(i, 1);
				}
			}
			for (size_t i = 0; i < str.size() - 1; ++i) {
				if (str[i] == '\\' && str[i + 1] == '\\') {
					str.erase(i, 1);
				}
			}
			names.push_back(str);
		}
	} else {
		Log("Path: " + path + " doesn't exists.");
	}
	return names;
}

pattern GetPatternsOrParams(std::string filename) {
	pattern p;
	std::ifstream ifs(filename);
	if (!ifs.is_open()) {
		Log("Can't open file " + filename);
		return p;
	}
	std::string line;
	while (std::getline(ifs, line)) {
		if (line.find("[{") != std::string::npos) {
			std::string patternName = line.substr(2, line.find("}]") - 2);
			std::vector<std::string> v;
			while (std::getline(ifs, line)) {
				if (line == "") {
					break;
				}
				Trim(line);
				v.push_back(line);
			}
			Trim(patternName);
			p[patternName] = v;
		}
	}
	return p;
}

location ParseLocations(std::vector<std::string>& files) {
	location loc;
	for (size_t i = 0; i < files.size(); i++) {
		std::ifstream ifs(files[i]);
		if (!ifs.is_open()) {
			Log("Can't open file " + files[i]);
			return loc;
		}
		std::string line;
		std::string locName;
		int curlyBraceCnt = 0;
		bool locArray = false;
		while (std::getline(ifs, line)) {
			if (StartsWith("#", line) || StartsWith("<%#", line)) {
				continue;
			}
			// found location array "<% %w("
			if (line.find("<% %w(") != std::string::npos) { 
				std::vector<std::string> locationsEnum; 
				locArray = true;
				while (std::getline(ifs, line)) { // read locations array
					Trim(line);
					if (line.find(").each do") != std::string::npos) { // if found end of location array - exit from cycle
						locArray = false;
					}					
					if (locArray) {
						locationsEnum.push_back(line);
					}
					if (line.find("location") != std::string::npos) { // found location
						Trim(line);
						locName = line;
						if (line.find("{") != std::string::npos) { // increasing counter of "{"
							curlyBraceCnt++;
						}
						std::vector<std::string> params;
						while (std::getline(ifs, line)) {		
							if (StartsWith("#", line) || StartsWith("<%#", line)) {
								continue;
							} else if (line.find("{") != std::string::npos) {
								curlyBraceCnt++;
							} else if (line.find("}") != std::string::npos) {
								if (curlyBraceCnt == 1) {
									curlyBraceCnt--;
									break;
								} else {
									curlyBraceCnt--;
								}
								Trim(line);
							} else if (line == "") {
							//} else if (line == "" || line == " " || line == "  " || line == "   " || line == "    ") {
								continue;
							}
							Trim(line);
							params.push_back(line);
						}
						// iterating through loc enum, constructing locName and filling locations
						for (size_t i = 0; i < locationsEnum.size(); i++) {
							std::string pattern = "<%= loc %>";
							Trim(locName);
							std::string newLocName = locName;							
							newLocName.replace(newLocName.find(pattern), pattern.size(), locationsEnum[i]); // subst locName instead of "<%= loc %>"
							newLocName = newLocName.substr(0, newLocName.find("{")); // removing { at the end of location line
							Trim(newLocName);
							if (loc[newLocName].first.size() != 0) {
								std::cout << "Duplicate location: " << newLocName;
								newLocName = "duplicate_" + newLocName;
							}
							loc[newLocName].first = params; 
						}
					}
					if (line.find("<% end %>") != std::string::npos) {
						break;
					}
				}			
			}
			//found "location"
			if (line.find("location") != std::string::npos) {
				std::vector<std::string> params;
				Trim(locName);
				locName = line.substr(0, line.find("{"));
				Trim(locName);
				if (line.find("{") != std::string::npos) { // increasing counter of "{"
					curlyBraceCnt++;
				}
				if (loc[locName].first.size() != 0) {
					std::cout << "Duplicate location: " << locName << std::endl;
					locName = "duplicate_" + locName;
				}
				while (std::getline(ifs, line)) {
					if (StartsWith("#", line) || StartsWith("<%#", line)) {
					//if (line.find("#") != std::string::npos) {
						continue;
					} else if (line.find("{") != std::string::npos) {
						curlyBraceCnt++;
					} else if (line.find("}") != std::string::npos) {
						if (curlyBraceCnt == 1) {
							curlyBraceCnt--;
							break;
						} else {
							curlyBraceCnt--;
						}
					} else if (line == "" || line == " " || line == "  " || line == "   " || line == "    ") {
						continue;
					}					
					Trim(line);
					loc[locName].first.push_back(line);
				}				
			}
		}
	}
	return loc;
}

void Group(pattern& patterns, location& locations) {
	for (const auto& pattern : patterns) {
		location matchedLoc;
		for (auto it = locations.begin(); it != locations.end(); ) {
			bool locMatch = true;

			if (pattern.first.find("strict") != std::string::npos) {
				if (pattern.second.size() != it->second.first.size()) {
					it++;
					continue;
				}				
				for (size_t i = 0; i < pattern.second.size(); ++i) {
					bool parameterMatch = false;
					for (size_t j = 0; j < it->second.first.size(); ++j) {
						if (CheckForInclude(it->second.first[j], pattern.second[i])) {
							parameterMatch = true; 
							break;
						}
					}
					if (!parameterMatch) {
						locMatch = false;
						it++;
						break;
					}
				}
			} else {
				if (pattern.second.size() > it->second.first.size()) {
					it++;
					continue;
				}
				for (size_t i = 0; i < pattern.second.size(); ++i) {
					bool parameterMatch = false;
					for (size_t j = 0; j < it->second.first.size(); ++j) {
						if (CheckForInclude(it->second.first[j], pattern.second[i])) {
							parameterMatch = true;
							break;
						}
					}
					if (!parameterMatch) {
						locMatch = false;
						it++;
						break;
					}
				}
			}
			if (locMatch) {
				matchedLoc.insert(*it); // adding fully matched location to matched loc map
				it = locations.erase(it); // erasing matched location
			}
		}
		SaveMatchedLocationsToFile(pattern, matchedLoc, "Locations.txt");
	}
}

void SaveK8sMatchedLocationsToFile(const std::pair<std::string, std::vector<std::string>>& pattern, const location& locations, std::string filename) {
	if (locations.size() == 0) {
		return;
	}
	if (!std::filesystem::is_directory("MatchedPatterns") || !std::filesystem::exists("MatchedPatterns")) {
		std::filesystem::create_directory("MatchedPatterns");
	}

	std::ofstream ofs(".\\MatchedPatterns\\" + filename, std::ios::app);
	if (!ofs.is_open()) {
		Log("Can't open file " + pattern.first);
		return;
	}
	ofs << pattern.first << ":\n";
	for (const auto& param : pattern.second) {
		ofs << "  - " << param << std::endl;
	}
	ofs << "\n  - " << "LOCATIONS:" << "\n";
	for (const auto& loc : locations) {
		ofs << "    - " << loc.first << "\n";
	}
	ofs << "\n";
}


void SaveMatchedLocationsToFile(const std::pair<std::string, std::vector<std::string>>& pattern, const location& locations, std::string filename) {
	if (locations.size() == 0) {
		return;
	}

	if (!std::filesystem::is_directory("MatchedPatterns") || !std::filesystem::exists("MatchedPatterns")) {
		std::filesystem::create_directory("MatchedPatterns");
	}

	std::ofstream ofs(".\\MatchedPatterns\\" + filename, std::ios::app);
	if (!ofs.is_open()) {
		Log("Can't open file " + pattern.first);
		return;
	}	
	ofs << pattern.first << ":\n";
	for (const auto& param : pattern.second) {
		if (param.find("proxy_pass") != std::string::npos) {
			continue;
		}
		ofs << "  - " << param << std::endl;
	}
	std::map<std::string, std::vector<std::string>> transform;
	for (const auto& loc : locations) {
		bool flag = false;
		for (size_t i = 0; i < loc.second.first.size(); ++i) {			
			if (loc.second.first[i].find("proxy_pass") != std::string::npos) {
				transform[loc.second.first[i]].push_back(loc.first);
				flag = true;
			}
		}
		if (!flag) {
			transform["LOCATIONS"].push_back(loc.first);
		}
	}
	ofs << "\n";
	for (auto& proxypass : transform) {
		ofs << "  - " << proxypass.first << ":\n";
		for (auto& loc : proxypass.second) {
			ofs << "    - " << loc << "\n";
		}
		ofs << "\n";
	}
}

void SaveUnmatchedLocationsToFile(const location& locations, std::string filename) {
	if (locations.size() == 0) {
		return;
	}

	std::ofstream ofs(filename);
	std::ofstream ofsdup("duplicate_" + filename);
	if (!ofs.is_open()) {
		Log("Can't open file " + filename);
		return;
	}
	if (!ofsdup.is_open()) {
		Log("Can't open file duplicate_" + filename);
		return;
	}
	for (const auto& loc : locations) {
		int curlyBracesCnt = 1; // starts from one because of added { in line with loc.first
		if (loc.first.find("duplicate_") != std::string::npos) {
			ofsdup << loc.first << " {" << std::endl;
			for (const auto& param : loc.second.first) {
				if (param.find("{") != std::string::npos) {
					curlyBracesCnt++;
				} else if (param.find("}") != std::string::npos) {
					curlyBracesCnt--;
				}
				for (size_t i = 0; i < curlyBracesCnt; ++i) {
					ofsdup << "  ";
				}
				ofsdup << param << std::endl;
			}
			ofsdup << "}" << std::endl;
			ofsdup << std::endl;
		} else {
			ofs << loc.first << " {" << std::endl;
			for (const auto& param : loc.second.first) {
				if (param.find("{") != std::string::npos) {
					curlyBracesCnt++;
				}
				else if (param.find("}") != std::string::npos) {
					curlyBracesCnt--;
				}
				for (size_t i = 0; i < curlyBracesCnt; ++i) {
					ofs << "  ";
				}
				ofs << param << std::endl;
			}
			ofs << "}" << std::endl;
			ofs << std::endl;
		}		
	}
}

void CleanUp() {	
	if (std::filesystem::exists("MatchedPatterns")) {
		std::filesystem::remove_all("MatchedPatterns");
	}
	if (std::filesystem::exists("UnmatchedLocations.txt")) {
		std::filesystem::remove("UnmatchedLocations.txt");
	}
	if (std::filesystem::exists("duplicate_UnmatchedLocations.txt")) {
		std::filesystem::remove("duplicate_UnmatchedLocations.txt");
	}
	if (std::filesystem::exists("LocationsList.txt")) {
		std::filesystem::remove("LocationsList.txt");
	}	
	if (std::filesystem::exists("NewParamsSliceByLocations.txt")) {
		std::filesystem::remove("NewParamsSliceByLocations.txt");
	}
}

bool CheckForInclude(const std::string& original, const std::string& pattern) {	
	if (pattern[0] == '&') { // & at start mean that match should be exact
		auto words = SplitIntoWords(pattern.substr(1, pattern.size() - 1));
		auto originalWords = SplitIntoWords(original);
		if (originalWords.size() < words.size()) {
			return false;
		}
		for (size_t i = 0; i < words.size(); ++i) {
			if (originalWords[i] != words[i]) {
				return false;
			}
		}
	} else {
		auto words = SplitIntoWords(pattern);
		for (size_t i = 0; i < words.size(); ++i) {
			if (original.find(words[i]) == std::string::npos) {
				return false;
			}
		}		
	}
	return true;
}

std::vector<std::string> SplitIntoWords(const std::string& s) {
	auto it = begin(s);
	std::vector<std::string> v;
	do {
		std::string str = "";
		while (it < end(s) && *it != ' ') {
			str += *it;
			it++;
		}
		if (!str.empty()) {
			v.push_back(str);
		}
		if (it < end(s)) {
			it++;
		}
	} while (it < end(s));
	return v;
}

std::unordered_set<std::string> GetK8sUpstreams(std::string filename) {
	std::unordered_set<std::string> upstreams;
	std::ifstream ifs(filename);
	if (!ifs.is_open()) {
		Log("Can't open file " + filename);
		return upstreams;
	}
	std::string line;
	while (std::getline(ifs, line)) {
		Trim(line);
		upstreams.insert(line);
	}
	return upstreams;
}

void GroupWithK8sUpstreams(pattern& patterns, location& locations, std::unordered_set<std::string>& upstreams) {
	for (const auto& pattern : patterns) {
		location matchedLoc;
		for (auto it = locations.begin(); it != locations.end(); ) {
			bool locMatch = false;

			for (auto& param : it->second.first) {
				if (param.find("proxy_pass") != std::string::npos) {
					std::string upstream = param.substr(param.find("http://") + 7, param.find(";") - param.find("http://") - 7);
					if (upstream[upstream.size() - 1] == '/') {
						upstream = upstream.substr(0, upstream.size() - 1);
					}
					if (upstreams.count(upstream) != 0) {
						locMatch = true;
						break;
					}
				}
			}
			if (!locMatch) {
				++it;
				continue;
			}

			if (pattern.first.find("strict") != std::string::npos) {
				if (pattern.second.size() != it->second.first.size()) {
					it++;
					continue;
				}
				for (size_t i = 0; i < pattern.second.size(); ++i) {
					bool parameterMatch = false;
					for (size_t j = 0; j < it->second.first.size(); ++j) {
						if (CheckForInclude(it->second.first[j], pattern.second[i])) {
							parameterMatch = true;
							break;
						}
					}
					if (!parameterMatch) {
						locMatch = false;
						it++;
						break;
					}
				}
			} else {
				if (pattern.second.size() > it->second.first.size()) {
					it++;
					continue;
				}
				for (size_t i = 0; i < pattern.second.size(); ++i) {
					bool parameterMatch = false;
					for (size_t j = 0; j < it->second.first.size(); ++j) {
						if (CheckForInclude(it->second.first[j], pattern.second[i])) {
							parameterMatch = true;
							break;
						}
					}
					if (!parameterMatch) {
						locMatch = false;
						it++;
						break;
					}
				}
			}
			if (locMatch) {				
				matchedLoc.insert(*it); // adding fully matched location to matched loc map
				it = locations.erase(it); // erasing matched location
			}
		}
		SaveK8sMatchedLocationsToFile(pattern, matchedLoc, "k8sLocations.txt");
	}

}

void AddParamsToLocations(location& locations, const newParams& newParams) {
	for (const auto& newParam : newParams) {
		std::string newParameter = newParam.first.substr(0, newParam.first.find("^^"));
		std::string newParameterValue = newParam.first.substr(newParam.first.find("^^") + 2, newParam.first.find("<<") - newParam.first.find("^^") - 2);
		std::string newParameterNewValue = newParam.first.substr(newParam.first.find("<<") + 2, newParam.first.find("__") - newParam.first.find("<<") - 2);
		std::string type = newParam.first.substr(newParam.first.find("__") + 2, newParam.first.size() - newParam.first.find("__"));
		
		
		if (type == "add") {
			for (auto& loc : newParam.second) {
				bool flag = false;
				try {
					for (auto& parameter : locations.at(loc).first) {
						
						if (parameter.find(newParameter) != std::string::npos) {
							flag = true;
						}
					}
					if (!flag) { // if newParameter not found - add
						std::string paramValue = newParameter + " " + newParameterValue; 
						Trim(paramValue); //sometimes there is need to use whole "parameter + value" as parameter and empty value, so its trim ws added above
						locations[loc].first.push_back(paramValue);
					}
				} catch (...) {}	// catching map.at() errors			
			}
		} else if (type == "replace") {
			for (auto& loc : newParam.second) {
				try {
					for (auto& parameter : locations.at(loc).first) {
						if (parameter.find(newParameter) != std::string::npos) {
							if (parameter.find(newParameterValue) != std::string::npos) {
								parameter.replace(parameter.find(newParameterValue), newParameterValue.size(), newParameterNewValue);
							}
						}
					}
				}
				catch (...) {}
			}
		} else if (type == "add_or_replace") {

		} else if (type == "delete") {
			for (auto& loc : newParam.second) {
				try {
					for (auto it = locations.at(loc).first.begin(); it != locations.at(loc).first.end(); ) {
						if ((*it).find(newParameter) != std::string::npos) {
							it = locations.at(loc).first.erase(it);
							continue;
						}
						++it;
					}
				}
				catch (...) {}
			}
		}
	}
}

void SaveLocations(location& locations, std::string filename) {
	if (locations.size() == 0) {
		return;
	}

	std::ofstream ofs(filename);
	if (!ofs.is_open()) {
		Log("Can't open file " + filename);
		return;
	}

	for (auto rIt = locations.rbegin(); rIt != locations.rend(); ++rIt) {
		ofs << rIt->first << "\n";
	}
}

void NewParamsSliceByLocationsAndSave(const newParams& newparams, std::string&& filename) {
	std::map<std::string, std::set<std::string>> ParamSliceByLocations;
	for (auto& param : newparams) {
		for (auto& loc : param.second) {
			std::string newParameter = param.first.substr(0, param.first.find("^^"));
			std::string newParameterValue = param.first.substr(param.first.find("^^") + 2, param.first.find("<<") - param.first.find("^^") - 2);
			std::string newParameterNewValue = param.first.substr(param.first.find("<<") + 2, param.first.find("__") - param.first.find("<<") - 2);
			std::string type = param.first.substr(param.first.find("__") + 2, param.first.size() - param.first.find("__"));
			if (type == "add") {
				ParamSliceByLocations[loc].insert("Added: " + newParameter + " " + newParameterValue);
			} else if (type == "replace") {
				ParamSliceByLocations[loc].insert("Replaced: " + newParameter + " " + newParameterValue + " -> " + newParameterNewValue);
			} else if (type == "delete") {
				ParamSliceByLocations[loc].insert("Deleted: " + newParameter + " " + newParameterValue + " -> " + newParameterNewValue);
			}
		}
	}
	if (ParamSliceByLocations.size() == 0) {
		return;		
	}
	
	std::ofstream ofs(filename);
	if (!ofs.is_open()) {
		Log("Can't open file " + filename);
		return;
	}
	for (auto& loc : ParamSliceByLocations) {
		ofs << loc.first << " {\n";
		for (auto& param : loc.second) {
			ofs << "\t" << param << "\n";
		}
		ofs << "}\n";
	}
}